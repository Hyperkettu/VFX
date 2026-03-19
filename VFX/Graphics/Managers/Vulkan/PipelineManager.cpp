#include "FoxRenderer.h"

// Static member definitions
std::unique_ptr<Fox::Graphics::Managers::Vulkan::PipelineManager> Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::PipelineManager>::instance = nullptr;
std::once_flag Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::PipelineManager>::initFlag;

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				bool PipelineManager::Initialize(VkDevice device, VkSurfaceCapabilitiesKHR capabilities) {
					
					{
						VkDescriptorSetLayout descriptorSetLayouts = Fox::Graphics::Managers::Vulkan::DescriptorManager::Get().GetDescriptorSet(Fox::Graphics::Managers::Vulkan::Descriptor::OFFSCREEN)->GetLayout().Get();

						std::unique_ptr<Fox::Graphics::Vulkan::PipelineLayout> offscreenPipelineLayout = std::make_unique<Fox::Graphics::Vulkan::PipelineLayout>(device,
							std::vector<VkDescriptorSetLayout>{ descriptorSetLayouts },
							std::vector<VkPushConstantRange>{});

						std::unique_ptr<Fox::Graphics::Vulkan::Pipeline> offscreenPipeline = std::make_unique<Fox::Graphics::Vulkan::Pipeline>(Fox::Graphics::Vulkan::PipelineBuilder(device)
							.WithShader(Fox::Graphics::Enums::PipelineShaderStage::MESH_SHADER, "Shaders/texturedCube.spv")
							.WithShader(Fox::Graphics::Enums::PipelineShaderStage::FRAGMENT_SHADER, "Shaders/textured_frag.spv")
							.WithViewport(0.0f, 0.0f, static_cast<float>(capabilities.currentExtent.width), static_cast<float>(capabilities.currentExtent.height), 0.0f, 1.0f)
							.WithScissor(0, 0, capabilities.currentExtent.width, capabilities.currentExtent.height)
							.SetLayout(offscreenPipelineLayout->Get())
							.SetRenderPass(Fox::Graphics::Managers::Vulkan::RenderPassManager::Get().GetPass(Fox::Graphics::Managers::Vulkan::RenderPass::DEFAULT)->Get())
							.Build());

						pipelines[Fox::Graphics::Managers::Vulkan::PipelineCategory::OFFSCREEN_RENDERING] = std::move(offscreenPipeline);
						pipelineLayouts[Fox::Graphics::Managers::Vulkan::PipelineCategory::OFFSCREEN_RENDERING] = std::move(offscreenPipelineLayout);

					}
					{

						VkDescriptorSetLayout descriptorSetLayouts = Fox::Graphics::Managers::Vulkan::DescriptorManager::Get().GetDescriptorSet(Fox::Graphics::Managers::Vulkan::Descriptor::MAIN_MESH_SHADER)->GetLayout().Get();

						auto mainPipelineLayout = std::make_unique<Fox::Graphics::Vulkan::PipelineLayout>(device,
							std::vector<VkDescriptorSetLayout>{ descriptorSetLayouts },
							std::vector<VkPushConstantRange>{});

						auto mainBuilder = Fox::Graphics::Vulkan::PipelineBuilder(device);

						std::unique_ptr<Fox::Graphics::Vulkan::Pipeline> mainMeshShaderPipeline = std::make_unique<Fox::Graphics::Vulkan::Pipeline>(mainBuilder
							.WithShader(Fox::Graphics::Enums::PipelineShaderStage::MESH_SHADER, "Shaders/multi_generic_mesh.spv")
							.WithShader(Fox::Graphics::Enums::PipelineShaderStage::FRAGMENT_SHADER, "Shaders/textured_multi_generic_mesh.spv")
							.WithViewport(0.0f, 0.0f, static_cast<float>(capabilities.currentExtent.width), static_cast<float>(capabilities.currentExtent.height), 0.0f, 1.0f)
							.WithScissor(0, 0, capabilities.currentExtent.width, capabilities.currentExtent.height)
							.SetLayout(mainPipelineLayout->Get())
							.SetRenderPass(Fox::Graphics::Managers::Vulkan::RenderPassManager::Get().GetPass(Fox::Graphics::Managers::Vulkan::RenderPass::DEFAULT)->Get())
							.Build());

						pipelines[Fox::Graphics::Managers::Vulkan::PipelineCategory::BASIC_MESH_SHADER] = std::move(mainMeshShaderPipeline);
						pipelineLayouts[Fox::Graphics::Managers::Vulkan::PipelineCategory::BASIC_MESH_SHADER] = std::move(mainPipelineLayout);

					}
					{

						VkDescriptorSetLayout descriptorSetLayouts = Fox::Graphics::Managers::Vulkan::DescriptorManager::Get().GetDescriptorSet(Fox::Graphics::Managers::Vulkan::Descriptor::MAIN_MESH_SHADER)->GetLayout().Get();

						auto mainPipelineLayout = std::make_unique<Fox::Graphics::Vulkan::PipelineLayout>(device,
							std::vector<VkDescriptorSetLayout>{ descriptorSetLayouts },
							std::vector<VkPushConstantRange>{});

						auto mainBuilder = Fox::Graphics::Vulkan::PipelineBuilder(device);

						std::unique_ptr<Fox::Graphics::Vulkan::Pipeline> mainMeshShaderPipeline = std::make_unique<Fox::Graphics::Vulkan::Pipeline>(mainBuilder
							.WithShader(Fox::Graphics::Enums::PipelineShaderStage::MESH_SHADER, "Shaders/multi_generic_mesh.spv")
							.WithShader(Fox::Graphics::Enums::PipelineShaderStage::FRAGMENT_SHADER, "Shaders/textured_multi_generic_mesh.spv")
							.WithViewport(0.0f, 0.0f, static_cast<float>(capabilities.currentExtent.width), static_cast<float>(capabilities.currentExtent.height), 0.0f, 1.0f)
							.WithScissor(0, 0, capabilities.currentExtent.width, capabilities.currentExtent.height)
							.SetLayout(mainPipelineLayout->Get())
							.SetRenderPass(Fox::Graphics::Managers::Vulkan::RenderPassManager::Get().GetPass(Fox::Graphics::Managers::Vulkan::RenderPass::DEFAULT_NO_CLEAR)->Get())
							.Build());

						pipelines[Fox::Graphics::Managers::Vulkan::PipelineCategory::BASIC_MESH_SHADER_NO_CLEAR] = std::move(mainMeshShaderPipeline);
						pipelineLayouts[Fox::Graphics::Managers::Vulkan::PipelineCategory::BASIC_MESH_SHADER_NO_CLEAR] = std::move(mainPipelineLayout);

					}

					{

						VkDescriptorSetLayout descriptorSetLayouts = Fox::Graphics::Managers::Vulkan::DescriptorManager::Get().GetDescriptorSet(Fox::Graphics::Managers::Vulkan::Descriptor::MAIN_MESH_BINDLESS_TEXTURING_SHADER)->GetLayout().Get();

						auto mainPipelineLayout = std::make_unique<Fox::Graphics::Vulkan::PipelineLayout>(device,
							std::vector<VkDescriptorSetLayout>{ descriptorSetLayouts },
							std::vector<VkPushConstantRange>{});

						auto mainBuilder = Fox::Graphics::Vulkan::PipelineBuilder(device);

						std::unique_ptr<Fox::Graphics::Vulkan::Pipeline> mainMeshBindlessShaderPipeline = std::make_unique<Fox::Graphics::Vulkan::Pipeline>(mainBuilder
							.WithShader(Fox::Graphics::Enums::PipelineShaderStage::MESH_SHADER, "Shaders/multi_material_generic_mesh.spv")
							.WithShader(Fox::Graphics::Enums::PipelineShaderStage::FRAGMENT_SHADER, "Shaders/textured_pbr_multi_generic_mesh.spv")
							.WithViewport(0.0f, 0.0f, static_cast<float>(capabilities.currentExtent.width), static_cast<float>(capabilities.currentExtent.height), 0.0f, 1.0f)
							.WithScissor(0, 0, capabilities.currentExtent.width, capabilities.currentExtent.height)
							.WithColorBlending(VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT |
								VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT, VK_TRUE, VK_FALSE, VkLogicOp::VK_LOGIC_OP_COPY, { 1.0f, 1.0f, 1.0f , 1.0f })
							.SetLayout(mainPipelineLayout->Get())
							.WithDepthStencil(VK_FALSE)
							.SetRenderPass(Fox::Graphics::Managers::Vulkan::RenderPassManager::Get().GetPass(Fox::Graphics::Managers::Vulkan::RenderPass::DEFAULT)->Get())
							.Build());

						pipelines[Fox::Graphics::Managers::Vulkan::PipelineCategory::MESH_SHADER_BINDLESS_TEXTURING] = std::move(mainMeshBindlessShaderPipeline);
						pipelineLayouts[Fox::Graphics::Managers::Vulkan::PipelineCategory::MESH_SHADER_BINDLESS_TEXTURING] = std::move(mainPipelineLayout);

					}

					{

						VkDescriptorSetLayout descriptorSetLayouts = Fox::Graphics::Managers::Vulkan::DescriptorManager::Get().GetDescriptorSet(Fox::Graphics::Managers::Vulkan::Descriptor::MAIN_MESH_BINDLESS_TEXTURING_SHADER)->GetLayout().Get();

						auto mainPipelineLayout = std::make_unique<Fox::Graphics::Vulkan::PipelineLayout>(device,
							std::vector<VkDescriptorSetLayout>{ descriptorSetLayouts },
							std::vector<VkPushConstantRange>{});

						auto mainBuilder = Fox::Graphics::Vulkan::PipelineBuilder(device);

						std::unique_ptr<Fox::Graphics::Vulkan::Pipeline> mainMeshBindlessShaderPipeline = std::make_unique<Fox::Graphics::Vulkan::Pipeline>(mainBuilder
							.WithShader(Fox::Graphics::Enums::PipelineShaderStage::MESH_SHADER, "Shaders/multi_material_generic_mesh.spv")
							.WithShader(Fox::Graphics::Enums::PipelineShaderStage::FRAGMENT_SHADER, "Shaders/textured_pbr_multi_generic_mesh.spv")
							.WithViewport(0.0f, 0.0f, static_cast<float>(capabilities.currentExtent.width), static_cast<float>(capabilities.currentExtent.height), 0.0f, 1.0f)
							.WithScissor(0, 0, capabilities.currentExtent.width, capabilities.currentExtent.height)
							.WithColorBlending(VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT |
								VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT, VK_TRUE, VK_FALSE, VkLogicOp::VK_LOGIC_OP_COPY, { 1.0f, 1.0f, 1.0f , 1.0f })
							.SetLayout(mainPipelineLayout->Get())
							.WithDepthStencil(VK_FALSE)
							.SetRenderPass(Fox::Graphics::Managers::Vulkan::RenderPassManager::Get().GetPass(Fox::Graphics::Managers::Vulkan::RenderPass::DEFAULT_NO_CLEAR)->Get())
							.Build());

						pipelines[Fox::Graphics::Managers::Vulkan::PipelineCategory::MESH_SHADER_BINDLESS_TEXTURING_NO_CLEAR] = std::move(mainMeshBindlessShaderPipeline);
						pipelineLayouts[Fox::Graphics::Managers::Vulkan::PipelineCategory::MESH_SHADER_BINDLESS_TEXTURING_NO_CLEAR] = std::move(mainPipelineLayout);

					}

						return true;
					
				}

			}
		}
	}
}