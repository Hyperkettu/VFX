#include "FoxRenderer.h"

namespace Fox {

	namespace Graphics {

		namespace Vulkan {

			VkShaderStageFlagBits ComputePipelineBuilder::ConvertShaderStage(Fox::Graphics::Enums::PipelineShaderStage stage) {
				switch (stage) {
				case Fox::Graphics::Enums::PipelineShaderStage::MESH_SHADER:
					return VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT;
				case Fox::Graphics::Enums::PipelineShaderStage::TASK_SHADER:
					return VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT;
				case Fox::Graphics::Enums::PipelineShaderStage::FRAGMENT_SHADER:
					return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
				case Fox::Graphics::Enums::PipelineShaderStage::VERTEX_SHADER:
					return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
				case Fox::Graphics::Enums::PipelineShaderStage::COMPUTE_SHADER:
					return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
				}

				return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
			}

			void ComputePipelineBuilder::DestroyShaderModules() {
				for (auto& module : shaderModules) {
					vkDestroyShaderModule(device, module, nullptr);
				}
				shaderModules.clear();
				shaderModuleCreateInfos.clear();
			}

			Fox::Graphics::Vulkan::ComputePipelineBuilder& ComputePipelineBuilder::WithShader(Fox::Graphics::Enums::PipelineShaderStage stage, const std::string& shaderPath) {
				std::vector<char> code = Fox::Core::FileSystem::ReadBinaryFile(shaderPath);
				VkShaderModule shaderModule;

				VkShaderModuleCreateInfo shaderModuleCreateInfo{};
				shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shaderModuleCreateInfo.codeSize = code.size();
				shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

				shaderModuleCreateInfos.push_back(shaderModuleCreateInfo);

				if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
					std::cout << "Failed to create shader module for shader" << std::endl;
					exit(1);
				}
				shaderModules.push_back(shaderModule);

				VkPipelineShaderStageCreateInfo shaderStageInfo{};
				shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageInfo.stage = Fox::Graphics::Vulkan::PipelineBuilder::ConvertShaderStage(stage);
				shaderStageInfo.module = shaderModule;
				shaderStageInfo.pName = "main";

				shaderStages.push_back(shaderStageInfo);

				return *this;
			}
		}
	}
}

