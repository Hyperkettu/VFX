#pragma once

#include <unordered_map>

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				enum class DepthTexture : int32_t {
					DEFAULT_DEPTH_STENCIL = 0,
				};

				enum class ShaderResourceTexture : int32_t {
					BOX = 0,
					KETTU,
					LUKKO
				};

				enum class RenderTargetTexture : int32_t {
					 DEFAULT_RENDER_TARGET = 0,
					 LAYER1_RENDER_TARGET,
					 LAYER2_RENDER_TARGET,
					 LAYER3_RENDER_TARGET,
					 LAYER4_RENDER_TARGET,
					 LAYER5_RENDER_TARGET,
					 POST_PROCESSING_TARGET
				};

				class TextureManager : public Fox::Core::Singleton<TextureManager> {
					friend class Singleton<Fox::Graphics::Managers::Vulkan::TextureManager>;

				public: 
					TextureManager() = default;
					~TextureManager() = default;
					
					bool Initialize(
						VkDevice device,
						VkPhysicalDevice physicalDevice,
						VkSurfaceCapabilitiesKHR capabilities,
						VkSurfaceFormatKHR surfaceFormat,
						VkFormat depthFormat,
						VkQueue graphicsQueue,
						uint32_t graphicsQueueFamily,
						std::unique_ptr<Fox::Graphics::Vulkan::CommandPool>& commandPool
						);

					void Destroy() {
						depthTextures.clear();
						shaderResourceTextures.clear();
						renderTargetTextures.clear();
					}

					void Update(uint32_t currentFrameIndex);

					inline std::unique_ptr<Fox::Graphics::Vulkan::DepthTexture>& GetDepthTexture(Fox::Graphics::Managers::Vulkan::DepthTexture depthTexture) {
						return depthTextures[depthTexture];
					}

					inline std::unique_ptr<Fox::Graphics::Vulkan::ShaderResourceTexture>& GetShaderResourceTexture(const std::string& shaderResourceTexture) {
						return shaderResourceTextures[shaderResourceTexture];
					}

					inline std::unique_ptr<Fox::Graphics::Vulkan::RenderTargetTexture>& GetRenderTargetTexture(Fox::Graphics::Managers::Vulkan::RenderTargetTexture renderTargetTexture) {
						return renderTargetTextures[renderTargetTexture];
					}

				private:

					VkDevice device;
					VkPhysicalDevice physicalDevice;
					VkSurfaceCapabilitiesKHR capabilities;
					VkSurfaceFormatKHR surfaceFormat;
					VkQueue graphicsQueue;
					uint32_t graphicsQueueFamily;

					std::unordered_map<Fox::Graphics::Managers::Vulkan::DepthTexture, std::unique_ptr<Fox::Graphics::Vulkan::DepthTexture>> depthTextures;
					std::unordered_map<std::string, std::unique_ptr<Fox::Graphics::Vulkan::ShaderResourceTexture>> shaderResourceTextures;
					std::unordered_map<Fox::Graphics::Managers::Vulkan::RenderTargetTexture, std::unique_ptr<Fox::Graphics::Vulkan::RenderTargetTexture>> renderTargetTextures;

					std::vector<std::string> split(const std::string& s, char delimiter) {
						std::vector<std::string> tokens;
						std::string token;
						std::stringstream ss(s);

						while (std::getline(ss, token, delimiter)) {
							tokens.push_back(token);
						}
						return tokens;
					}
				};
			}
		}
	}
}