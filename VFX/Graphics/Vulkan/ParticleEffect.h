#pragma once 

namespace Fox {

	namespace Graphics {
	
		namespace Vulkan {
	
			class ParticleEffect {
			public:
				ParticleEffect() = default;
				virtual ~ParticleEffect() = default;

				virtual void Initialize(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t maxParticles, uint32_t maxFramesInFlight) = 0;

				virtual void Update(std::unique_ptr<Fox::Graphics::Vulkan::CommandPool>& commandPool, std::unique_ptr<Fox::Graphics::Vulkan::CommandList>& commandList, VkQueue computeQueue, uint32_t frameIndex, float dt) = 0;
				virtual void Render(std::unique_ptr<Fox::Graphics::Vulkan::CommandList>& commandList, uint32_t imageIndex, uint32_t frameIndex, VkSurfaceCapabilitiesKHR capabilities, std::array<VkClearValue, 2>& clearValues, bool clear) = 0;

			public: 

				uint32_t maxParticles;
				std::vector<std::unique_ptr<Fox::Graphics::Vulkan::DescriptorSet>> particleUpdateDescriptorSets;
				std::vector < std::unique_ptr<Fox::Graphics::Vulkan::DescriptorSet>> particleRenderDescriptorSets;
			};
		}
	
	}

}