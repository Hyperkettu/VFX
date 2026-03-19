#pragma once

#include "VFX/Graphics/Geometry/Material.h"
#include "VFX/Graphics/Geometry/MaterialInstance.h"

namespace Fox 
{
    namespace Scene {
        class Scene;
    }

    namespace Graphics {

        namespace Managers {
            namespace Vulkan {
                enum class SceneId;
            }
        }

        namespace Vulkan {

            struct FrameResource
            {
                struct SceneConstantBuffers {
                    std::unique_ptr<Fox::Graphics::Vulkan::ConstantBuffer<Fox::Graphics::Vulkan::PerFrame>> perFrameUBO;
                    std::unique_ptr<Fox::Graphics::Vulkan::DynamicConstantBuffer<glm::mat4>> meshTransformsUBO;
                    std::unique_ptr<Fox::Graphics::Vulkan::DynamicBuffer<Fox::Graphics::Vulkan::MeshInfo>> meshInfosUBO;    
                    std::unique_ptr<Fox::Graphics::Vulkan::DynamicConstantBuffer<Fox::Geometry::AlbedoMaterial>> materialsUBO;
                    std::unique_ptr<Fox::Graphics::Vulkan::DescriptorSet> perFrameDescriptorSet;
                };

				FrameResource() = default; 

                int32_t CreateGraphicsCommandResources(VkDevice device, uint32_t queueFamily);
                int32_t CreateSynchronizationObjects(VkDevice device, bool createFenceAsSignaled);
                void Destroy(VkDevice device);

                int32_t CreateConstantBuffers(VkDevice device, VkPhysicalDevice physicalDevice, std::vector<std::shared_ptr<Fox::Scene::Scene>>& activeScenes);

                Fox::Graphics::Vulkan::FrameResource::SceneConstantBuffers& GetSceneConstantBuffers(Fox::Graphics::Managers::Vulkan::SceneId id) {
                    return sceneConstantBuffers[id];
                }

                // --- Synchronization ---
                std::unique_ptr<Fox::Graphics::Vulkan::Fence> renderFence; // Fence: signals when frame is done rendering
                std::unique_ptr<Fox::Graphics::Vulkan::Semaphore> imageAvailableSemaphore;// Signals when swapchain image is ready
                std::unique_ptr<Fox::Graphics::Vulkan::Semaphore>  renderFinishedSemaphore;// Signals when rendering is finished
                std::unique_ptr<Fox::Graphics::Vulkan::Semaphore> offscreenFinishedSemaphore;

                // --- Command recording ---
                std::unique_ptr<Fox::Graphics::Vulkan::CommandPool> commandPool;         // Per-frame command pool
                std::unique_ptr<Fox::Graphics::Vulkan::CommandList> commandList;      // Main primary command buffer
                std::unique_ptr<Fox::Graphics::Vulkan::CommandList> offscreenCommandList;

                // --- Descriptor management ---
                std::unique_ptr<Fox::Graphics::Vulkan::DescriptorSet> offscreenDescriptorSet;

                // --- Per-frame uniform/constant data ---
                std::unique_ptr<Fox::Graphics::Vulkan::ConstantBuffer<Fox::Graphics::Vulkan::OldFrame>> oldPerFrameUBO;
                std::unordered_map<Fox::Graphics::Managers::Vulkan::SceneId, Fox::Graphics::Vulkan::FrameResource::SceneConstantBuffers> sceneConstantBuffers;

                // --- Dynamic or staging buffers ---
                //Buffer stagingBuffer;               // optional: per-frame upload staging buffer

                // --- Frame index tracking ---
                uint32_t frameIndex;
            };
        }
    }
}

