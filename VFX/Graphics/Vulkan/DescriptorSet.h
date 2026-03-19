#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

namespace Fox {

    namespace Graphics
    {

        namespace Vulkan {

            class DescriptorSet {
            public:
                DescriptorSet() = default;

                DescriptorSet(VkDevice device,
                    VkDescriptorPool pool,
                    VkDescriptorSetLayout layout, 
                    uint32_t maxTextures)
                    : device(device), layout(layout)
                {

                    VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
                    countInfo.sType =
                        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    countInfo.descriptorSetCount = 1;
                    countInfo.pDescriptorCounts = &maxTextures;

                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = pool;
                    allocInfo.descriptorSetCount = 1;
                    allocInfo.pSetLayouts = &layout;

                    if (maxTextures > 0) {
                        allocInfo.pNext = &countInfo;
                    }

                    if (vkAllocateDescriptorSets(device, &allocInfo, &set) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to allocate Vulkan descriptor set!");
                    }
                }

                DescriptorSet(const DescriptorSet&) = delete;
                DescriptorSet& operator=(const DescriptorSet&) = delete;

                DescriptorSet(DescriptorSet&& other) noexcept
                    : device(other.device), layout(other.layout), set(other.set)
                {
                    other.set = VK_NULL_HANDLE;
                }

                DescriptorSet& operator=(DescriptorSet&& other) noexcept {
                    if (this != &other) {
                        device = other.device;
                        layout = other.layout;
                        set = other.set;
                        other.set = VK_NULL_HANDLE;
                    }
                    return *this;
                }

                ~DescriptorSet() = default; // freed when pool is destroyed

                inline VkDescriptorSet Get() const { return set; }
                VkDescriptorSetLayout GetLayout() const { return layout; }

				void SetShaderResourceTexture(uint32_t slotIndex, const Fox::Graphics::Vulkan::Texture* texture) {
					VkWriteDescriptorSet descriptorWrites{};

					VkDescriptorImageInfo imageInfo{};
					imageInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = texture->GetView();
					imageInfo.sampler = texture->GetSampler();
                    imageInfos.push_back(imageInfo);

					descriptorWrites.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrites.dstSet = set;
					descriptorWrites.dstBinding = slotIndex;
					descriptorWrites.dstArrayElement = 0;
					descriptorWrites.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrites.descriptorCount = 1;
					descriptorWrites.pImageInfo = &imageInfos[imageInfos.size() -1];

					writes.push_back(descriptorWrites);
				}

                void SetBindlessTextureArray(uint32_t slotIndex, const std::vector<Fox::Graphics::Vulkan::Texture*> textures) {
                    VkWriteDescriptorSet descriptorWrites{};
                    bindlessImageInfos.clear();

                    for (auto* texturePtr : textures) {

                        Fox::Graphics::Vulkan::Texture* texture = texturePtr;

                        if(!texture || texture->GetView() == VK_NULL_HANDLE || texture->GetSampler() == VK_NULL_HANDLE) {
                            std::cout << "Invalid texture provided to SetBindlessTextureArray" << std::endl;
							std::cout << "Replacing with default texture" << std::endl;
							texture = Fox::Graphics::Managers::Vulkan::TextureManager::Get().GetShaderResourceTexture("default").get();
						}

                        VkDescriptorImageInfo imageInfo{};
                        imageInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        imageInfo.imageView = texture->GetView();
                        imageInfo.sampler = texture->GetSampler();
                        bindlessImageInfos.push_back(imageInfo);
                    }

                    descriptorWrites.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites.dstSet = set;
                    descriptorWrites.dstBinding = slotIndex;
                    descriptorWrites.dstArrayElement = 0;
                    descriptorWrites.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites.descriptorCount = bindlessImageInfos.size();
                    descriptorWrites.pImageInfo = bindlessImageInfos.data();

                    writes.push_back(descriptorWrites);
                }

				void ClearWrites() {
					writes.clear();
                    bufferInfos.clear();
                    imageInfos.clear();
                    bindlessImageInfos.clear();
                }

				template<class T>
                void SetConstantBuffer(uint32_t slotIndex, std::unique_ptr<Fox::Graphics::Vulkan::ConstantBuffer<T>>& buffer) {
                    VkWriteDescriptorSet descriptorWrites{};

                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo = buffer->DescriptorInfo();
                    bufferInfos.push_back(bufferInfo);

                    descriptorWrites.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites.dstSet = set;
                    descriptorWrites.dstBinding = slotIndex;
                    descriptorWrites.dstArrayElement = 0;
                    descriptorWrites.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrites.descriptorCount = 1;
                    descriptorWrites.pBufferInfo = &bufferInfos[bufferInfos.size() - 1];

					writes.push_back(descriptorWrites);
				}

                template<class T>
                void SetDynamicConstantBuffer(uint32_t slotIndex, std::unique_ptr<Fox::Graphics::Vulkan::DynamicConstantBuffer<T>>& buffer) {
                    VkWriteDescriptorSet descriptorWrites{};

                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo = buffer->DescriptorInfo();
                    bufferInfos.push_back(bufferInfo);

                    descriptorWrites.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites.dstSet = set;
                    descriptorWrites.dstBinding = slotIndex;
                    descriptorWrites.dstArrayElement = 0;
                    descriptorWrites.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrites.descriptorCount = 1;
                    descriptorWrites.pBufferInfo = &bufferInfos[bufferInfos.size() - 1];

                    writes.push_back(descriptorWrites);
                }

                void Reserve(uint32_t maxDescriptors) {
                    writes.reserve(maxDescriptors);
                    bufferInfos.reserve(maxDescriptors);
                    imageInfos.reserve(maxDescriptors);
                }

				template<class T>
                void SetDynamicStorageBuffer(uint32_t slotIndex, std::unique_ptr<Fox::Graphics::Vulkan::DynamicBuffer<T>>& buffer) {
                    VkWriteDescriptorSet descriptorWrite{};

                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo = buffer->DescriptorInfo();
					bufferInfos.push_back(bufferInfo);

                    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite.dstSet = set;
                    descriptorWrite.dstBinding = slotIndex;
                    descriptorWrite.dstArrayElement = 0;
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrite.descriptorCount = 1;
                    descriptorWrite.pBufferInfo = &bufferInfos.back();
                    descriptorWrite.pImageInfo = nullptr;
                    descriptorWrite.pTexelBufferView = nullptr;

                    writes.push_back(descriptorWrite);
                }

                void SetStorageBuffer(uint32_t slotIndex, std::unique_ptr<Fox::Graphics::Vulkan::Buffer>& buffer) {
                    VkWriteDescriptorSet descriptorWrite{};

                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = buffer->Get();
                    bufferInfo.offset = 0;
                    bufferInfo.range = buffer->GetSize();
                    bufferInfos.push_back(bufferInfo);

                    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite.dstSet = set;
                    descriptorWrite.dstBinding = slotIndex;
                    descriptorWrite.dstArrayElement = 0;
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrite.descriptorCount = 1;
                    descriptorWrite.pBufferInfo = &bufferInfos.back();
                    descriptorWrite.pImageInfo = nullptr;
                    descriptorWrite.pTexelBufferView = nullptr;

                    writes.push_back(descriptorWrite);
                }

                template<class T>
                void SetVertexBuffer(uint32_t slotIndex, std::unique_ptr<Fox::Graphics::Vulkan::VertexBuffer>& buffer) {
                    VkWriteDescriptorSet descriptorWrite{};

                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = buffer->Get();
                    bufferInfo.offset = 0;
                    bufferInfo.range = buffer->GetVertexCount() * sizeof(T);
                    bufferInfos.push_back(bufferInfo);

                    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite.dstSet = set;
                    descriptorWrite.dstBinding = slotIndex;
                    descriptorWrite.dstArrayElement = 0;
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrite.descriptorCount = 1;
                    descriptorWrite.pBufferInfo = &bufferInfos[bufferInfos.size() - 1];;
                    descriptorWrite.pImageInfo = nullptr;
                    descriptorWrite.pTexelBufferView = nullptr;

                    writes.push_back(descriptorWrite);
                }

                template<class T>
                void SetIndexBuffer(uint32_t slotIndex, std::unique_ptr<Fox::Graphics::Vulkan::IndexBuffer>& buffer) {
                    VkWriteDescriptorSet descriptorWrite{};

                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = buffer->Get();
                    bufferInfo.offset = 0;
                    bufferInfo.range = buffer->GetIndexCount() * sizeof(T);
                    bufferInfos.push_back(bufferInfo);

                    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite.dstSet = set;
                    descriptorWrite.dstBinding = slotIndex;
                    descriptorWrite.dstArrayElement = 0;
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrite.descriptorCount = 1;
                    descriptorWrite.pBufferInfo = &bufferInfos[bufferInfos.size() - 1];;
                    descriptorWrite.pImageInfo = nullptr;
                    descriptorWrite.pTexelBufferView = nullptr;

                    writes.push_back(descriptorWrite);
                }

                void Update() {
                    Update(writes);
                }

                void Update(const std::vector<VkWriteDescriptorSet>& writes) const {
                    vkUpdateDescriptorSets(device,
                        static_cast<uint32_t>(writes.size()),
                        writes.data(),
                        0, nullptr);
                }

            private:
                VkDevice device = VK_NULL_HANDLE;
                VkDescriptorSetLayout layout = VK_NULL_HANDLE;
                VkDescriptorSet set = VK_NULL_HANDLE;

				std::vector<VkWriteDescriptorSet> writes;
                std::vector<VkDescriptorBufferInfo> bufferInfos;
                std::vector<VkDescriptorImageInfo> imageInfos;

                std::vector<VkDescriptorImageInfo> bindlessImageInfos;

            };

        }
    }
}