#pragma once

#include <unordered_map>

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				enum class RenderPass : int32_t {
					DEFAULT = 0,
					DEFAULT_NO_CLEAR,
					OFFSCREEN,
					OFFSCREEN_NO_CLEAR,
					TO_PRESENTABLE,
					TO_PRESENTABLE_NO_CLEAR
				};

				class RenderPassManager : public Fox::Core::Singleton<RenderPassManager> {
					friend class Singleton <Fox::Graphics::Managers::Vulkan::RenderPassManager>;

				public: 
					RenderPassManager() = default;
					~RenderPassManager() = default;

					bool Initialize(VkDevice device, VkSurfaceCapabilitiesKHR capabilities, VkSurfaceFormatKHR surfaceFormat, VkFormat depthFormat);

					void Destroy() {
						renderPasses.clear();
					}

					std::unique_ptr<Fox::Graphics::Vulkan::RenderPass>& GetPass(Fox::Graphics::Managers::Vulkan::RenderPass pass) {
						return renderPasses[pass];
					}

				private:
					std::unordered_map<Fox::Graphics::Managers::Vulkan::RenderPass, std::unique_ptr<Fox::Graphics::Vulkan::RenderPass>> renderPasses;
				};
			}
		}
	}
}