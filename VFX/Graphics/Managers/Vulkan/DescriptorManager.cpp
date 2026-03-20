#include "FoxRenderer.h"

// Static member definitions
std::unique_ptr<Fox::Graphics::Managers::Vulkan::DescriptorManager> Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::DescriptorManager>::instance = nullptr;
std::once_flag Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::DescriptorManager>::initFlag;

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				bool DescriptorManager::Initialize(VkDevice device, const Fox::Graphics::RendererConfig& config, uint32_t sceneCount) {
					{
						auto mainMeshPass = std::make_unique<Fox::Graphics::Vulkan::DescriptorSetBuilder>(Fox::Graphics::Vulkan::DescriptorSetBuilder(device));
						mainMeshPass
							->AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
							.SetMaxSets(config.MAX_FRAMES_IN_FLIGHT * sceneCount)
							.Build(); 

						descriptorSetBuilders[Fox::Graphics::Managers::Vulkan::Descriptor::MAIN_MESH_SHADER] = std::move(mainMeshPass);
						
					} 

					{
						auto mainMeshBindlessPass = std::make_unique<Fox::Graphics::Vulkan::DescriptorSetBuilder>(Fox::Graphics::Vulkan::DescriptorSetBuilder(device));
						mainMeshBindlessPass
							->AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, Fox::Graphics::Managers::Vulkan::MaterialManager::NUM_TEXTURES_PER_BINDLESS_ARRAY)
							.SetMaxSets(config.MAX_FRAMES_IN_FLIGHT * sceneCount)
							.SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
							.Build();

						descriptorSetBuilders[Fox::Graphics::Managers::Vulkan::Descriptor::MAIN_MESH_BINDLESS_TEXTURING_SHADER] = std::move(mainMeshBindlessPass);

					}

					{
						auto offscreenDescriptorSets = std::make_unique<Fox::Graphics::Vulkan::DescriptorSetBuilder>(Fox::Graphics::Vulkan::DescriptorSetBuilder(device));
						offscreenDescriptorSets
							->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
							.SetMaxSets(config.MAX_FRAMES_IN_FLIGHT)
							.Build();

						descriptorSetBuilders[Fox::Graphics::Managers::Vulkan::Descriptor::OFFSCREEN] = std::move(offscreenDescriptorSets);
					}

					{
						auto postProcessorDescriptorSets = std::make_unique<Fox::Graphics::Vulkan::DescriptorSetBuilder>(Fox::Graphics::Vulkan::DescriptorSetBuilder(device));
						postProcessorDescriptorSets
							->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
							.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
							.AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
							.SetMaxSets(config.MAX_FRAMES_IN_FLIGHT)
							.Build();

						descriptorSetBuilders[Fox::Graphics::Managers::Vulkan::Descriptor::POST_PROCESSOR] = std::move(postProcessorDescriptorSets);

					}

					{
						auto particleUpdateDescriptorSets = std::make_unique<Fox::Graphics::Vulkan::DescriptorSetBuilder>(Fox::Graphics::Vulkan::DescriptorSetBuilder(device));
						particleUpdateDescriptorSets
							->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT | VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT)
							.AddBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT)
							.SetMaxSets(config.MAX_FRAMES_IN_FLIGHT)
							.Build();

						descriptorSetBuilders[Fox::Graphics::Managers::Vulkan::Descriptor::EULER_PARTICLE_UPDATE] = std::move(particleUpdateDescriptorSets);
					}

					{
						auto particleRenderDescriptorSets = std::make_unique<Fox::Graphics::Vulkan::DescriptorSetBuilder>(Fox::Graphics::Vulkan::DescriptorSetBuilder(device));
						particleRenderDescriptorSets
							->AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT)
							.SetMaxSets(config.MAX_FRAMES_IN_FLIGHT)
							.Build();

						descriptorSetBuilders[Fox::Graphics::Managers::Vulkan::Descriptor::EULER_PARTICLE_RENDER] = std::move(particleRenderDescriptorSets);
					}

					return true;
				}

			}
		}
	}
}