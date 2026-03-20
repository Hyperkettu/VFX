#pragma once 

namespace Fox {

	namespace Graphics {
	
		namespace Vulkan {
		
			class EulerParticleEffect : public Fox::Graphics::Vulkan::ParticleEffect {
			public:
				EulerParticleEffect() = default;
				virtual ~EulerParticleEffect() = default;

				virtual void Update(std::unique_ptr<Fox::Graphics::Vulkan::CommandPool>& commandPool, std::unique_ptr<Fox::Graphics::Vulkan::CommandList>& commandList, VkQueue computeQueue, uint32_t frameIndex, float dt) override;
				virtual void Render(std::unique_ptr<Fox::Graphics::Vulkan::CommandList>& commandList, uint32_t imageIndex, uint32_t frameIndex, VkSurfaceCapabilitiesKHR capabilities, std::array<VkClearValue, 2>& clearValues, bool clear) override;

				virtual void Initialize(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t maxParticles, uint32_t maxFramesInFlight) override;

				void UpdateConstantBuffersReferences(uint32_t frameIndex, uint32_t maxFramesInFlight);

			private: 
				VkDevice device;
				VkPhysicalDevice physicalDevice;

				std::vector<std::unique_ptr<Fox::Graphics::Vulkan::ConstantBuffer<Fox::Graphics::Vulkan::EulerParticleUpdate>>> particleUpdateConstantBuffers;
				std::vector<std::unique_ptr<Fox::Graphics::Vulkan::DynamicBuffer<Fox::Graphics::Vulkan::EulerParticle>>> particleBuffers;
				std::vector<std::unique_ptr<Fox::Graphics::Vulkan::DynamicConstantBuffer<glm::mat4>>> particleWorldTransforms;
			
			};
		}
	
	}

}