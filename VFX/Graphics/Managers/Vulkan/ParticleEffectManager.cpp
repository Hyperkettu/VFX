#include "FoxRenderer.h"

// Static member definitions
std::unique_ptr<Fox::Graphics::Managers::Vulkan::ParticleEffectManager> Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::ParticleEffectManager>::instance = nullptr;
std::once_flag Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::ParticleEffectManager>::initFlag;

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				bool ParticleEffectManager::Initialize(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t maxFramesInFlight) {
					this->device = device;
					this->maxFramesInFlight = maxFramesInFlight;
					this->physicalDevice = physicalDevice;

					InitializeSyncObjects(maxFramesInFlight);


					auto basicEulerEffect = std::make_unique<Fox::Graphics::Vulkan::EulerParticleEffect>();
					basicEulerEffect->Initialize(device, physicalDevice, 4096, maxFramesInFlight);
					particleEffects[Fox::Graphics::Managers::Vulkan::ParticleEffect::EULER_EFFECT] = std::move(basicEulerEffect);

					return true;
				}

				void ParticleEffectManager::Update(std::unique_ptr<Fox::Graphics::Vulkan::CommandPool>& commandPool, std::unique_ptr<Fox::Graphics::Vulkan::CommandList>& commandList, VkQueue computeQueue, uint32_t currentFrame, float dt) {
					particleUpdateFences[currentFrame]->Wait();
					particleUpdateFences[currentFrame]->Reset();

					commandPool->Reset();

					for (auto& [key, particleEffect] : particleEffects) {
						particleEffect->Update(commandPool, commandList, computeQueue, currentFrame, dt);
					}

					commandList->Submit(computeQueue, nullptr, particleUpdateFinishedSemaphores[currentFrame]->Get(), particleUpdateFences[currentFrame]->Get());
				}

				void ParticleEffectManager::Render(std::unique_ptr<Fox::Graphics::Vulkan::CommandPool>& commandPool, 
					std::unique_ptr<Fox::Graphics::Vulkan::CommandList>& commandList, 
					VkQueue graphicsQueue,
					std::unique_ptr<Fox::Graphics::Vulkan::Semaphore>& imageAvailableSemaphore,
					uint32_t currentFrame, uint32_t imageIndex, VkSurfaceCapabilitiesKHR capabilities,
					std::array<VkClearValue, 2> clearValues) {

					particleRenderFences[currentFrame]->Wait();
					particleRenderFences[currentFrame]->Reset();

					commandList->Begin();

					uint32_t index = 0;
					for (auto& [key, particleEffect] : particleEffects) {

						bool clear = index == 0;
						particleEffect->Render(commandList, imageIndex, currentFrame, capabilities, clearValues, clear);
						index++;
					}

					commandList->End();

					std::vector<VkSemaphore> waitSemaphores = { imageAvailableSemaphore->Get(), particleUpdateFinishedSemaphores[currentFrame]->Get() };
					std::vector<VkPipelineStageFlags> waitStages = { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };
					commandList->Submit(graphicsQueue, waitSemaphores, particleRenderingFinishedSemaphores[currentFrame]->Get(), particleRenderFences[currentFrame]->Get(), waitStages);

				}

				void ParticleEffectManager::InitializeSyncObjects(uint32_t maxFramesInFlight) {
					particleUpdateFences.resize(maxFramesInFlight);
					particleRenderFences.resize(maxFramesInFlight);
					particleUpdateFinishedSemaphores.resize(maxFramesInFlight);
					particleRenderingFinishedSemaphores.resize(maxFramesInFlight);

					for (auto i = 0; i < maxFramesInFlight; i++) {
						particleUpdateFences[i] = std::make_unique<Fox::Graphics::Vulkan::Fence>(device, true);
						particleRenderFences[i] = std::make_unique<Fox::Graphics::Vulkan::Fence>(device, true);
						particleUpdateFinishedSemaphores[i] = std::make_unique<Fox::Graphics::Vulkan::Semaphore>(device);
						particleRenderingFinishedSemaphores[i] = std::make_unique<Fox::Graphics::Vulkan::Semaphore>(device);
					}
				}

			}
		}
	}
}