#include "FoxRenderer.h"

#include "VFX\Core\Utils.h"

namespace Fox {

	namespace Graphics {

		namespace Vulkan {

			void EulerParticleEffect::Initialize(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t maxParticles, uint32_t maxFramesInFlight) {
				this->device = device;
				this->physicalDevice = physicalDevice;
				this->maxParticles = maxParticles;
				particleUpdateDescriptorSets.resize(maxFramesInFlight);
				particleRenderDescriptorSets.resize(maxFramesInFlight);
				particleUpdateConstantBuffers.resize(maxFramesInFlight);
				particleBuffers.resize(maxFramesInFlight);
				particleWorldTransforms.resize(maxFramesInFlight);

				// Initial particle positions on a circle
				float speed = 0.5f;
				std::vector<Fox::Graphics::Vulkan::EulerParticle> particles(maxParticles);
				for (auto& particle : particles) {
					float r = 0.25f * sqrt(Fox::randomFloat(0.0f, 1.0f));
					float theta = Fox::randomFloat(0.0f, 1.0f) * 2 * 3.14159265358979323846f;
					float x = r * cos(theta);
					float y = r * sin(theta);
					particle.position = glm::vec3(x, y, 0.0f);
					particle.velocity = glm::normalize(glm::vec3(x, y, 0.0f)) * speed;
					particle.color = glm::vec4(Fox::randomFloat(0.0f, 1.0f), Fox::randomFloat(0.0f, 1.0f), Fox::randomFloat(0.0f, 1.0f), 1.0f);
					particle.life = 0.0f;
					particle.maxLifetime = Fox::randomFloat(0.5f, 2.0f);
				}

				for (auto i = 0; i < maxFramesInFlight; i++) {
					particleUpdateDescriptorSets[i] = std::make_unique<Fox::Graphics::Vulkan::DescriptorSet>(
						Fox::Graphics::Managers::Vulkan::DescriptorManager::Get().GetDescriptorSet(Fox::Graphics::Managers::Vulkan::Descriptor::EULER_PARTICLE_UPDATE)
						->AllocateSet());
					// uniform buffer and 2 storage buffers
					particleUpdateDescriptorSets[i]->Reserve(3);

					particleUpdateConstantBuffers[i] = std::make_unique<Fox::Graphics::Vulkan::ConstantBuffer<Fox::Graphics::Vulkan::EulerParticleUpdate>>(device, physicalDevice);
					particleBuffers[i] = std::make_unique<Fox::Graphics::Vulkan::DynamicBuffer<Fox::Graphics::Vulkan::EulerParticle>>(device, physicalDevice);
					particleBuffers[i]->Update(particles);

					particleRenderDescriptorSets[i] = std::make_unique<Fox::Graphics::Vulkan::DescriptorSet>(Fox::Graphics::Managers::Vulkan::DescriptorManager::Get()
						.GetDescriptorSet(Fox::Graphics::Managers::Vulkan::Descriptor::EULER_PARTICLE_RENDER)->AllocateSet());
					particleRenderDescriptorSets[i]->Reserve(3);

					particleWorldTransforms[i] = std::make_unique<Fox::Graphics::Vulkan::DynamicConstantBuffer<glm::mat4>>(device, physicalDevice);

					glm::mat4 world = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
					std::vector<glm::mat4> worldTransforms = { world };
					particleWorldTransforms[i]->Update(worldTransforms);
				}

				for (auto i = 0; i < maxFramesInFlight; i++) {
					UpdateConstantBuffersReferences(i, maxFramesInFlight);
				}


			}

			void EulerParticleEffect::Update(std::unique_ptr<Fox::Graphics::Vulkan::CommandPool>& commandPool, std::unique_ptr<Fox::Graphics::Vulkan::CommandList>& commandList, VkQueue computeQueue, uint32_t frameIndex, float dt) {
				//UpdateConstantBuffersReferences(frameIndex);
				Fox::Graphics::Vulkan::EulerParticleUpdate particleUpdate;
				particleUpdate.deltaTime = dt;
				particleUpdateConstantBuffers[frameIndex]->Update(particleUpdate);

				commandList->Begin().
					BindPipeline(Fox::Graphics::Managers::Vulkan::PipelineManager::Get()
						.GetPipeline(Fox::Graphics::Managers::Vulkan::PipelineCategory::EULER_PARTICLE_UPDATE_COMPUTE_PIPELINE)->Get(),
						VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE)
					.BindDescriptorSets(Fox::Graphics::Managers::Vulkan::PipelineManager::Get().GetPipeline(Fox::Graphics::Managers::Vulkan::PipelineCategory::EULER_PARTICLE_UPDATE_COMPUTE_PIPELINE)->GetLayout(), 0,
						{ particleUpdateDescriptorSets[frameIndex]->Get() }, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE)
					.DispatchComputeShader(maxParticles / 256, 1, 1)
					.End();

				glm::mat4 world = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 15.0f));
				std::vector<glm::mat4> worldTransforms = { world };
				particleWorldTransforms[frameIndex]->Update(worldTransforms);
			}

			void EulerParticleEffect::Render(std::unique_ptr<Fox::Graphics::Vulkan::CommandList>& commandList, uint32_t imageIndex, uint32_t frameIndex, VkSurfaceCapabilitiesKHR capabilities, std::array<VkClearValue, 2>& clearValues, bool clear) {
				
				commandList->BeginRenderPass(
					Fox::Graphics::Managers::Vulkan::RenderPassManager::Get().GetPass(clear ? Fox::Graphics::Managers::Vulkan::RenderPass::DEFAULT : Fox::Graphics::Managers::Vulkan::RenderPass::DEFAULT_NO_CLEAR)->Get(),
					Fox::Graphics::Managers::Vulkan::FrameResourceManager::Get().GetSwapchain()->GetFramebuffer(imageIndex),
					capabilities.currentExtent,
					clearValues.data(),
					2)
					.SetViewport(0, 0, capabilities.currentExtent.width, capabilities.currentExtent.height)
					.SetScissor(0, 0, capabilities.currentExtent.width, capabilities.currentExtent.height)
					.BindPipeline(Fox::Graphics::Managers::Vulkan::PipelineManager::Get().GetPipeline(clear ? 
						Fox::Graphics::Managers::Vulkan::PipelineCategory::EULER_PARTICLE_RENDER : 
						Fox::Graphics::Managers::Vulkan::PipelineCategory::EULER_PARTICLE_RENDER_NO_CLEAR)->Get())
					.BindDescriptorSets(Fox::Graphics::Managers::Vulkan::PipelineManager::Get().GetPipelineLayout(clear ? Fox::Graphics::Managers::Vulkan::PipelineCategory::EULER_PARTICLE_RENDER :
						Fox::Graphics::Managers::Vulkan::PipelineCategory::EULER_PARTICLE_RENDER_NO_CLEAR)->Get(), 0,
						{ particleRenderDescriptorSets[frameIndex]->Get() })
					.RenderMeshShader(maxParticles / 32, 1, 1)
					.EndRenderPass();
			}

			void EulerParticleEffect::UpdateConstantBuffersReferences(uint32_t frameIndex, uint32_t maxFramesInFlight) {
				auto& updateSet = particleUpdateDescriptorSets[frameIndex];

				updateSet->ClearWrites();
				updateSet->SetConstantBuffer<Fox::Graphics::Vulkan::EulerParticleUpdate>(0, particleUpdateConstantBuffers[frameIndex]);
				updateSet->SetDynamicStorageBuffer<Fox::Graphics::Vulkan::EulerParticle>(1, particleBuffers[(frameIndex - 1) % maxFramesInFlight]); // last frame
				updateSet->SetDynamicStorageBuffer<Fox::Graphics::Vulkan::EulerParticle>(2, particleBuffers[frameIndex]); // current frame

				updateSet->Update();

				auto& renderSet = particleRenderDescriptorSets[frameIndex];
				renderSet->ClearWrites();
				renderSet->SetDynamicStorageBuffer<Fox::Graphics::Vulkan::EulerParticle>(0, particleBuffers[frameIndex]);
				renderSet->SetConstantBuffer<Fox::Graphics::Vulkan::PerFrame>(1, Fox::Graphics::Managers::Vulkan::FrameResourceManager::Get().GetFrameResource(frameIndex)
					->GetSceneConstantBuffers(Fox::Graphics::Managers::Vulkan::SceneId::MAIN_SCENE).perFrameUBO);
				renderSet->SetDynamicConstantBuffer<glm::mat4>(2, particleWorldTransforms[frameIndex]);

				renderSet->Update();
			}

		}

	}

}