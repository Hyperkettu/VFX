#include "FoxRenderer.h"

#include "VFX/Scene/Scene.h"


namespace Fox {

    namespace Graphics {

        namespace Vulkan {

            int32_t FrameResource::CreateGraphicsCommandResources(VkDevice device, uint32_t queueFamily) {
                commandPool = std::make_unique<Fox::Graphics::Vulkan::CommandPool>(device, queueFamily);
				commandList = std::make_unique<Fox::Graphics::Vulkan::CommandList>(device, commandPool->Get());
                offscreenCommandList = std::make_unique<Fox::Graphics::Vulkan::CommandList>(device, commandPool->Get());
                return 0;
            }

            int32_t FrameResource::CreateSynchronizationObjects(VkDevice device, bool createFenceAsSignaled) {
				imageAvailableSemaphore = std::make_unique<Fox::Graphics::Vulkan::Semaphore>(device);
				renderFinishedSemaphore = std::make_unique<Fox::Graphics::Vulkan::Semaphore>(device);   
                offscreenFinishedSemaphore = std::make_unique<Fox::Graphics::Vulkan::Semaphore>(device);
                renderFence = std::make_unique<Fox::Graphics::Vulkan::Fence>(device, createFenceAsSignaled);
                return 1;
            }

            void FrameResource::Destroy(VkDevice device) {
                sceneConstantBuffers.clear();

                oldPerFrameUBO = nullptr;
                commandList = nullptr;
                offscreenCommandList = nullptr;
				commandPool = nullptr;
                offscreenDescriptorSet = nullptr;
				imageAvailableSemaphore = nullptr;
				renderFinishedSemaphore = nullptr;  
                offscreenFinishedSemaphore = nullptr;
				renderFence = nullptr;
            }

            int32_t FrameResource::CreateConstantBuffers(VkDevice device, VkPhysicalDevice physicalDevice, std::vector<std::shared_ptr<Fox::Scene::Scene>>& activeScenes) {

                for (auto& scene : activeScenes) {
                    sceneConstantBuffers[scene->GetID()] = {
                        std::make_unique<Fox::Graphics::Vulkan::ConstantBuffer<Fox::Graphics::Vulkan::PerFrame>>(device, physicalDevice),
                        std::make_unique<Fox::Graphics::Vulkan::DynamicConstantBuffer<glm::mat4>>(device, physicalDevice),
                        std::make_unique<Fox::Graphics::Vulkan::DynamicBuffer<Fox::Graphics::Vulkan::MeshInfo>>(device, physicalDevice),
                        std::make_unique<Fox::Graphics::Vulkan::DynamicConstantBuffer<Fox::Geometry::AlbedoMaterial>>(device, physicalDevice)
                    };
                }

                oldPerFrameUBO = std::make_unique<Fox::Graphics::Vulkan::ConstantBuffer<Fox::Graphics::Vulkan::OldFrame>>(device, physicalDevice);
                return 1;
            }
        }
    }
}