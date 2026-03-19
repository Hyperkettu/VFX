#pragma once

#include <unordered_map>

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				enum class FrameBuffer : int32_t {
					OFFSCREEN_TARGET = 0,
					OFFSCREEN_TARGET_LAYER1,
					OFFSCREEN_TARGET_LAYER2,
					OFFSCREEN_TARGET_LAYER3,
					OFFSCREEN_TARGET_LAYER4,
					OFFSCREEN_TARGET_LAYER5,
					POST_PROCESSING_WITH_CLEAR,
					POST_PROCESSING_TARGET_NO_CLEAR,
					POST_PROCESSING_TO_SCREEN
				};

				class FrameResourceManager : public Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::FrameResourceManager> {
					friend class Singleton <Fox::Graphics::Managers::Vulkan::FrameResourceManager>;

				public:
					FrameResourceManager() = default;
					~FrameResourceManager() = default;

					bool Initialize(VkDevice device, const Fox::Graphics::RendererConfig& config, VkSurfaceCapabilitiesKHR capabilities, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkFormat depthFormat, int32_t graphicsQueueFamily);

					int32_t CreateUniformBuffers(const Fox::Graphics::RendererConfig& config, VkPhysicalDevice physicalDevice);

					void UpdateConstantBuffersReferences(uint32_t frameIndex, Fox::Scene::Scene* scene, bool updateBindless);

					template<class T, class E>
					uint32_t UpdateConstantBuffersForScene(uint32_t frameIndex, Fox::Scene::Scene* scene);

					void Destroy() {

						for (uint32_t i = 0u; i < frameResources.size(); i++)
						{
							frameResources[i]->Destroy(device);
						}

						frameResources.clear();
						frameBuffers.clear();
						swapchain = nullptr;
					}

					inline std::unique_ptr<Fox::Graphics::Vulkan::FrameResource>& GetFrameResource(uint32_t frameIndex) {
						return frameResources[frameIndex];
					}

					inline std::unique_ptr<Fox::Graphics::Vulkan::Swapchain>& GetSwapchain() {
						return swapchain;
					}

					inline std::unique_ptr < Fox::Graphics::Vulkan::Framebuffer>& GetFramebuffer(Fox::Graphics::Managers::Vulkan::FrameBuffer framebufferId) {
						return frameBuffers[framebufferId];
					}

					float rotationAngle = 0.0f;

				private:
					VkDevice device;

					std::vector<std::unique_ptr<Fox::Graphics::Vulkan::FrameResource>> frameResources;
					std::unique_ptr<Fox::Graphics::Vulkan::Swapchain> swapchain;
					std::unordered_map<Fox::Graphics::Managers::Vulkan::FrameBuffer, std::unique_ptr<Fox::Graphics::Vulkan::Framebuffer>> frameBuffers;
				};
			}
		}
	}
}