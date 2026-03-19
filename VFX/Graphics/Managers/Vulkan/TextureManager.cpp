#include "FoxRenderer.h"

#include "VFX/Core/Utils.h"

// Static member definitions
std::unique_ptr<Fox::Graphics::Managers::Vulkan::TextureManager> Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::TextureManager>::instance = nullptr;
std::once_flag Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::TextureManager>::initFlag;

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				bool TextureManager::Initialize(
					VkDevice device,
					VkPhysicalDevice physicalDevice,
					VkSurfaceCapabilitiesKHR capabilities,
					VkSurfaceFormatKHR surfaceFormat,
					VkFormat depthFormat,
					VkQueue graphicsQueue,
					uint32_t graphicsQueueFamily,
					std::unique_ptr<Fox::Graphics::Vulkan::CommandPool>& commandPool
				) {
					this->device = device;
					this->physicalDevice = physicalDevice;
					this->capabilities = capabilities;
					this->surfaceFormat = surfaceFormat;
					this->graphicsQueue = graphicsQueue;
					this->graphicsQueueFamily = graphicsQueueFamily;

					{
						VkExtent3D extent3D = {
							capabilities.currentExtent.width,
							capabilities.currentExtent.height,
							1
						};

						auto defaultDepthStencilTexture = std::make_unique<Fox::Graphics::Vulkan::DepthTexture>(device, physicalDevice, extent3D, depthFormat);

						depthTextures[Fox::Graphics::Managers::Vulkan::DepthTexture::DEFAULT_DEPTH_STENCIL] = std::move(defaultDepthStencilTexture);
					}

					std::string data = Fox::Core::FileSystem::ReadFile("Config/textures.txt");
					auto lines = split(data, '\n');

					for (auto& line : lines) {
						auto textureName = split(line, ' ')[0];
						auto path = split(line, ' ')[1];

						std::cout << "Texture: " << textureName << " in path " << path << " loaded!" << std::endl;

						auto texture = Fox::Graphics::Vulkan::ShaderResourceTexture::LoadFromFile(device,
							physicalDevice,
							commandPool->Get(),
							graphicsQueue,
							path
						);

						texture->SetName(textureName);

						shaderResourceTextures[textureName] = std::make_unique<Fox::Graphics::Vulkan::ShaderResourceTexture>(std::move(*texture));
					}
					{
						auto renderTargetTexture = std::make_unique<Fox::Graphics::Vulkan::RenderTargetTexture>(
							device,
							physicalDevice,
							VkExtent3D{ capabilities.currentExtent.width, capabilities.currentExtent.height, 1 },
							surfaceFormat.format,
							VK_IMAGE_ASPECT_COLOR_BIT);

						renderTargetTextures[Fox::Graphics::Managers::Vulkan::RenderTargetTexture::DEFAULT_RENDER_TARGET] = std::move(renderTargetTexture);

						for(auto i = 0u; i < 5; i++) {
							auto renderTargetTexture = std::make_unique<Fox::Graphics::Vulkan::RenderTargetTexture>(
								device,
								physicalDevice,
								VkExtent3D{ capabilities.currentExtent.width, capabilities.currentExtent.height, 1 },
								surfaceFormat.format,
								VK_IMAGE_ASPECT_COLOR_BIT);
							renderTargetTextures[static_cast<Fox::Graphics::Managers::Vulkan::RenderTargetTexture>(i + 1)] = std::move(renderTargetTexture);
						}

						auto postProcessingTarget = std::make_unique<Fox::Graphics::Vulkan::RenderTargetTexture>(
							device,
							physicalDevice,
							VkExtent3D{ capabilities.currentExtent.width, capabilities.currentExtent.height, 1 },
							surfaceFormat.format,
							VK_IMAGE_ASPECT_COLOR_BIT);

						renderTargetTextures[Fox::Graphics::Managers::Vulkan::RenderTargetTexture::POST_PROCESSING_TARGET] = std::move(postProcessingTarget);
					}

					return true;
				}

				void TextureManager::Update(uint32_t currentFrameIndex) {

				}
			}
		}

	}
}