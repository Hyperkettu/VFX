#pragma once

#include <unordered_map>
#include "VFX/Graphics/Vulkan/ParticleEffect.h"

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				enum class ParticleEffect : int32_t {
					EULER_EFFECT = 0
				};

				class ParticleEffectManager : public Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::ParticleEffectManager> {  
					friend class Singleton <Fox::Graphics::Managers::Vulkan::MeshManager>;

				public: 

					ParticleEffectManager() = default;
					~ParticleEffectManager() = default;

					bool Initialize(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t maxFramesInFlight);

					void Destroy() {
						particleEffects.clear();

						particleUpdateFences.clear();
						particleUpdateFinishedSemaphores.clear();
						particleRenderingFinishedSemaphores.clear();
						particleRenderFences.clear();

					}

					void Update(std::unique_ptr<Fox::Graphics::Vulkan::CommandPool>& commandPool, std::unique_ptr<Fox::Graphics::Vulkan::CommandList>& commandList, VkQueue computeQueue, uint32_t currentFrame, float dt);
					void Render(std::unique_ptr<Fox::Graphics::Vulkan::CommandPool>& commandPool, 
						std::unique_ptr<Fox::Graphics::Vulkan::CommandList>& commandList, 
						VkQueue graphicsQueue, 
						std::unique_ptr<Fox::Graphics::Vulkan::Semaphore>& imageAvailableSemaphore, 
						uint32_t currentFrame,
						uint32_t imageIndex, 
						VkSurfaceCapabilitiesKHR capabilities,
						std::array<VkClearValue, 2> clearValues);

					inline std::unique_ptr<Fox::Graphics::Vulkan::ParticleEffect>& GetParticleEffect(Fox::Graphics::Managers::Vulkan::ParticleEffect id) {
						return particleEffects[id];
					}

					VkSemaphore GetParticlesFinishedSemaphore(uint32_t currentFrame) {
						return particleRenderingFinishedSemaphores[currentFrame]->Get();
					}

					Fox::Graphics::Vulkan::Fence* GetParticlesFinishedFence(uint32_t index) {
						return particleRenderFences[index].get();
					}

				private:

					VkDevice device;
					VkPhysicalDevice physicalDevice;

					uint32_t maxFramesInFlight;

					void InitializeSyncObjects(uint32_t maxFramesInFlight);

					std::unordered_map<Fox::Graphics::Managers::Vulkan::ParticleEffect, std::unique_ptr<Fox::Graphics::Vulkan::ParticleEffect>> particleEffects;

					std::vector<std::unique_ptr<Fox::Graphics::Vulkan::Fence>> particleUpdateFences;
					std::vector<std::unique_ptr<Fox::Graphics::Vulkan::Fence>> particleRenderFences;
					std::vector<std::unique_ptr<Fox::Graphics::Vulkan::Semaphore>> particleUpdateFinishedSemaphores;
					std::vector <std::unique_ptr<Fox::Graphics::Vulkan::Semaphore>> particleRenderingFinishedSemaphores;
				};
			}
		}
	}
}