#pragma once

// Minimal includes
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <iostream>

// FFmpeg
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

namespace Fox {

    namespace Graphics {

        namespace Vulkan {

            class VulkanVideoPlayer {
            public:
                VulkanVideoPlayer() = default;

                VulkanVideoPlayer(VkDevice device,
                    VkPhysicalDevice physical,
                    VkQueue graphicsQueue,
                    uint32_t graphicsQueueFamily,
                    VkCommandPool uploadCmdPool)
                    : device(device),
                    physicalDevice(physical),
                    queue(graphicsQueue),
                    queueFamily(graphicsQueueFamily),
                    cmdPool(uploadCmdPool)
                {
                    avformat_network_init();
                }

                ~VulkanVideoPlayer() {
                    Cleanup();
                    avformat_network_deinit();
                }

                void SetFPS(float newFPS) {
                    fps = newFPS;
				}

                void UpdateNextFrame(bool saveToDisk) {
                    if (saveToDisk) {
                        float frameDurationInFrames = 1440.0f / static_cast<float>(fps);
                        nextFrameIndex += frameDurationInFrames;
                    }
                    else {
                        float frameDurationInSeconds = 60.0f / static_cast<float>(fps);
                        nextFrameIndex += frameDurationInSeconds;
                    }
                }

                bool ShouldUpdateTexture(uint32_t currentFrameIndex) {
                    return nextFrameIndex <= currentFrameIndex;
                }

                void SetCommandPool(std::unique_ptr<Fox::Graphics::Vulkan::CommandPool>& newCmdPool) {
                    videoCommandPool = std::move(newCmdPool);
				}

				uint32_t GetHeight() const { return height; }
				uint32_t GetWidth() const { return width; }
				int GetFPS() const { return fps; }

                // Initialize and open file. Returns true on success.
                bool Initialize(const std::string& filename) {
                    CleanupFFmpeg(); // in case

                    // Open input
                    if (avformat_open_input(&fmtCtx, filename.c_str(), nullptr, nullptr) < 0) {
                        std::cerr << "Failed to open input: " << filename << "\n";
                        return false;
                    }
                    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
                        std::cerr << "Failed to find stream info\n";
                        return false;
                    }

                    // Find the first video stream
                    videoStreamIndex = -1;
                    for (unsigned i = 0; i < fmtCtx->nb_streams; ++i) {
                        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                            videoStreamIndex = static_cast<int>(i);
                            break;
                        }
                    }
                    if (videoStreamIndex < 0) {
                        std::cerr << "No video stream found\n";
                        return false;
                    }

                    AVStream* vs = fmtCtx->streams[videoStreamIndex];

                    // Find decoder
                    const AVCodec* codec = avcodec_find_decoder(vs->codecpar->codec_id);
                    if (!codec) {
                        std::cerr << "Unsupported codec\n";
                        return false;
                    }

                    codecCtx = avcodec_alloc_context3(codec);
                    if (!codecCtx) {
                        std::cerr << "Failed to allocate codec context\n";
                        return false;
                    }

                    if (avcodec_parameters_to_context(codecCtx, vs->codecpar) < 0) {
                        std::cerr << "Failed to copy codec params\n";
                        return false;
                    }

                    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
                        std::cerr << "Failed to open codec\n";
                        return false;
                    }

                    // Prepare frames + scaler (we convert to RGBA)
                    pkt = av_packet_alloc();
                    frame = av_frame_alloc();
                    swsCtx = sws_getContext(
                        codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
                        codecCtx->width, codecCtx->height, AV_PIX_FMT_RGBA,
                        SWS_BILINEAR, nullptr, nullptr, nullptr);
                    if (!swsCtx) {
                        std::cerr << "Failed to create sws context\n";
                        return false;
                    }

                    // Allocate buffer for converted frame
                    rgbaLinesize = 0;
                    av_image_alloc(rgbaFrameData, rgbaFrameLinesize, codecCtx->width, codecCtx->height, AV_PIX_FMT_RGBA, 1);
                    // note: av_image_alloc returns linesize in rgbaFrameLinesize[0]

                    // Create Vulkan image to hold RGBA data
                    width = codecCtx->width;
                    height = codecCtx->height;
                    imageFormat = VK_FORMAT_R8G8B8A8_UNORM;

                    CreateVideoImage();

                    return true;
                }

                // Decode next packet/frame and upload to GPU image.
                // Returns true if a frame was decoded+uploaded, false if EOF or error.
                bool DecodeNextFrame() {
                    if (!fmtCtx || !codecCtx) return false;

                    int ret = 0;
                    while (true) {
                        // Read packet
                        ret = av_read_frame(fmtCtx, pkt);
                        if (ret < 0) {
                            // EOF or error
                            // Try to flush decoder by sending null packet
                            av_packet_unref(pkt);
                            avcodec_send_packet(codecCtx, nullptr);
                            // fall through to receive frames if any
                        }
                        else {
                            if (pkt->stream_index != videoStreamIndex) {
                                av_packet_unref(pkt);
                                continue; // skip non-video
                            }
                            // send packet
                            ret = avcodec_send_packet(codecCtx, pkt);
                            av_packet_unref(pkt);
                            if (ret < 0) {
                                std::cerr << "Error sending packet to decoder\n";
                                return false;
                            }
                        }

                        // receive frame(s)
                        ret = avcodec_receive_frame(codecCtx, frame);
                        if (ret == AVERROR(EAGAIN)) {
                            // Need more data; continue reading packets
                            if (av_packet_unref(pkt), ret < 0) continue;
                            continue;
                        }
                        else if (ret < 0) {
                            // EOF or error
                            return false;
                        }

                        // Convert to RGBA
                        uint8_t* dstPlanes[4] = { rgbaFrameData[0], nullptr, nullptr, nullptr };
                        int dstLineSize[4] = { rgbaFrameLinesize[0], 0, 0, 0 };

                        sws_scale(swsCtx,
                            frame->data,
                            frame->linesize,
                            0,
                            height,
                            dstPlanes,
                            dstLineSize);

                        // Upload to GPU image
                        UploadRGBAFrameToImage(rgbaFrameData[0], rgbaFrameLinesize[0]);

                        // Done with this decoded frame
                        av_frame_unref(frame);
                        return true;
                    }
                }

                // Returns the image view that contains the current decoded texture.
                VkImageView GetImageView() const { return videoTexture->GetView(); }

				std::unique_ptr<Fox::Graphics::Vulkan::ShaderResourceTexture>& GetTexture() { return videoTexture; }

                // Clean up everything
                void Cleanup() {
                    CleanupVulkanResources();
                    CleanupFFmpeg();
					videoCommandPool = nullptr; 
                }

            private:
                // --- Vulkan context (provided) ---
                VkDevice device = VK_NULL_HANDLE;
                VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
                VkQueue queue = VK_NULL_HANDLE;
                uint32_t queueFamily = 0;
                VkCommandPool cmdPool = VK_NULL_HANDLE;

                // --- FFmpeg objects ---
                AVFormatContext* fmtCtx = nullptr;
                AVCodecContext* codecCtx = nullptr;
                AVPacket* pkt = nullptr;
                AVFrame* frame = nullptr;
                SwsContext* swsCtx = nullptr;
                int videoStreamIndex = -1;

                // RGBA convert buffer
                uint8_t* rgbaFrameData[4] = { nullptr, nullptr, nullptr, nullptr };
                int rgbaFrameLinesize[4] = { 0, 0, 0, 0 };
                int rgbaLinesize = 0;

                float fps = 0;
                int nextFrameIndex = 0;

				std::unique_ptr<Fox::Graphics::Vulkan::CommandPool> videoCommandPool = nullptr;

      
            //    VkImage image = VK_NULL_HANDLE;
            //    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
            //    VkImageView imageView = VK_NULL_HANDLE;
                VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;

				std::unique_ptr<Fox::Graphics::Vulkan::ShaderResourceTexture> videoTexture = nullptr;
                uint32_t width = 0, height = 0;

                // helper staging buffer
                struct BufferAndMemory {
                    VkBuffer buf = VK_NULL_HANDLE;
                    VkDeviceMemory mem = VK_NULL_HANDLE;
                };

                // --- helpers ---
                void CleanupFFmpeg() {
                    if (rgbaFrameData[0]) {
                        av_freep(&rgbaFrameData[0]);
                        rgbaFrameLinesize[0] = 0;
                    }
                    if (swsCtx) { sws_freeContext(swsCtx); swsCtx = nullptr; }
                    if (frame) { av_frame_free(&frame); frame = nullptr; }
                    if (pkt) { av_packet_free(&pkt); pkt = nullptr; }
                    if (codecCtx) { avcodec_free_context(&codecCtx); codecCtx = nullptr; }
                    if (fmtCtx) { avformat_close_input(&fmtCtx); fmtCtx = nullptr; }
                }

                void CleanupVulkanResources() {
                    videoTexture = nullptr;
                }

                void CreateVideoImage() {
                    if (videoTexture != nullptr) {
                        CleanupVulkanResources();
                    }

                    videoTexture = std::make_unique<Fox::Graphics::Vulkan::ShaderResourceTexture>(
                        device,
                        physicalDevice,
                        VkExtent3D{ width, height, 1 },
                        imageFormat,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VK_IMAGE_ASPECT_COLOR_BIT
                    );

               /*     VkImageCreateInfo imgCi{};
                    imgCi.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                    imgCi.imageType = VK_IMAGE_TYPE_2D;
                    imgCi.format = imageFormat;
                    imgCi.extent = { width, height, 1 };
                    imgCi.mipLevels = 1;
                    imgCi.arrayLayers = 1;
                    imgCi.samples = VK_SAMPLE_COUNT_1_BIT;
                    imgCi.tiling = VK_IMAGE_TILING_OPTIMAL;
                    imgCi.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                    imgCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    imgCi.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

                    if (vkCreateImage(device, &imgCi, nullptr, &image) != VK_SUCCESS)
                        throw std::runtime_error("Failed to create video image");

                    VkMemoryRequirements memReq;
                    vkGetImageMemoryRequirements(device, image, &memReq);

                    uint32_t memTypeIndex = FindMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                    VkMemoryAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    allocInfo.allocationSize = memReq.size;
                    allocInfo.memoryTypeIndex = memTypeIndex;

                    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
                        throw std::runtime_error("Failed to allocate image memory");

                    vkBindImageMemory(device, image, imageMemory, 0);

                    // create view
                    VkImageViewCreateInfo viewCi{};
                    viewCi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    viewCi.image = image;
                    viewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    viewCi.format = imageFormat;
                    viewCi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    viewCi.subresourceRange.levelCount = 1;
                    viewCi.subresourceRange.layerCount = 1;

                    if (vkCreateImageView(device, &viewCi, nullptr, &imageView) != VK_SUCCESS)
                        throw std::runtime_error("Failed to create image view"); */
                }

                // Find memory type helper
                uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) {
                    VkPhysicalDeviceMemoryProperties memProps;
                    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
                    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
                        if ((typeFilter & (1u << i)) && (memProps.memoryTypes[i].propertyFlags & props) == props)
                            return i;
                    }
                    throw std::runtime_error("Failed to find suitable memory type");
                }

                // Upload RGBA pixels to the GPU image (synchronous; records a short cmd buffer, submits and waits).
                void UploadRGBAFrameToImage(const uint8_t* srcPixels, int srcRowPitch) {
                    // Create staging buffer and copy pixel rows into it (tightly packed)
                    const VkDeviceSize imageSize = static_cast<VkDeviceSize>(srcRowPitch) * height;

                    BufferAndMemory staging = CreateBuffer(imageSize,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                    // Map and copy
                    void* mapped = nullptr;
                    vkMapMemory(device, staging.mem, 0, imageSize, 0, &mapped);

                    // FFmpeg RGBA might have stride > width*4, copy line by line
                    for (uint32_t y = 0; y < height; ++y) {
                        const uint8_t* srcRow = srcPixels + size_t(y) * srcRowPitch;
                        uint8_t* dstRow = reinterpret_cast<uint8_t*>(mapped) + size_t(y) * (width * 4);
                        memcpy(dstRow, srcRow, size_t(width) * 4);
                    }
                    vkUnmapMemory(device, staging.mem);

                    // Record command buffer: transition image -> copy buffer-> image -> transition to SHADER_READ_ONLY_OPTIMAL
                    VkCommandBuffer cmd = BeginSingleTimeCommands();

                    // transition to transfer dst
                    TransitionImageLayout(cmd, videoTexture->GetImage(), VK_FORMAT_R8G8B8A8_UNORM,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

                    // copy buffer to image
                    VkBufferImageCopy region{};
                    region.bufferOffset = 0;
                    region.bufferRowLength = 0;   // tightly packed
                    region.bufferImageHeight = 0;
                    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.imageSubresource.mipLevel = 0;
                    region.imageSubresource.baseArrayLayer = 0;
                    region.imageSubresource.layerCount = 1;
                    region.imageOffset = { 0, 0, 0 };
                    region.imageExtent = { width, height, 1 };

                    vkCmdCopyBufferToImage(cmd, staging.buf, videoTexture->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

                    // transition to shader read
                    TransitionImageLayout(cmd, videoTexture->GetImage(), VK_FORMAT_R8G8B8A8_UNORM,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                    EndSingleTimeCommands(cmd);

                    // cleanup staging
                    vkDestroyBuffer(device, staging.buf, nullptr);
                    vkFreeMemory(device, staging.mem, nullptr);
                }

                // Create buffer helper
                BufferAndMemory CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
                    BufferAndMemory out{};

                    VkBufferCreateInfo bufCi{};
                    bufCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                    bufCi.size = size;
                    bufCi.usage = usage;
                    bufCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                    if (vkCreateBuffer(device, &bufCi, nullptr, &out.buf) != VK_SUCCESS)
                        throw std::runtime_error("Failed to create buffer");

                    VkMemoryRequirements memReq;
                    vkGetBufferMemoryRequirements(device, out.buf, &memReq);

                    VkMemoryAllocateInfo alloc{};
                    alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    alloc.allocationSize = memReq.size;
                    alloc.memoryTypeIndex = FindMemoryType(memReq.memoryTypeBits, properties);

                    if (vkAllocateMemory(device, &alloc, nullptr, &out.mem) != VK_SUCCESS)
                        throw std::runtime_error("Failed to allocate buffer memory");

                    vkBindBufferMemory(device, out.buf, out.mem, 0);
                    return out;
                }

                // Begin a single-time command buffer, submit and end helpers
                VkCommandBuffer BeginSingleTimeCommands() {
                    VkCommandBufferAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                    allocInfo.commandPool = cmdPool;
                    allocInfo.commandBufferCount = 1;

                    VkCommandBuffer cmd;
                    vkAllocateCommandBuffers(device, &allocInfo, &cmd);

                    VkCommandBufferBeginInfo beginInfo{};
                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                    vkBeginCommandBuffer(cmd, &beginInfo);
                    return cmd;
                }

                void EndSingleTimeCommands(VkCommandBuffer cmd) {
                    vkEndCommandBuffer(cmd);
                    VkSubmitInfo submit{};
                    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                    submit.commandBufferCount = 1;
                    submit.pCommandBuffers = &cmd;

                    vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE);
                    vkQueueWaitIdle(queue);

                    vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
                }

                // Transition image layout helper
                void TransitionImageLayout(VkCommandBuffer cmd, VkImage img, VkFormat fmt, VkImageLayout oldLayout, VkImageLayout newLayout) {
                    VkImageMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.oldLayout = oldLayout;
                    barrier.newLayout = newLayout;
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = img;
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier.subresourceRange.baseMipLevel = 0;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.baseArrayLayer = 0;
                    barrier.subresourceRange.layerCount = 1;

                    VkPipelineStageFlags srcStage = 0;
                    VkPipelineStageFlags dstStage = 0;

                    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                        barrier.srcAccessMask = 0;
                        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    }
                    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    }
                    else {
                        // general case (add others as needed)
                        barrier.srcAccessMask = 0;
                        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    }

                    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
                }
            };
        }
    }
}