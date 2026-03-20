#pragma once

#include "VFX/Graphics/Enums/PipelineShaderStage.h"

namespace Fox
{
    namespace Graphics
    {
        namespace Vulkan
        {
            class ComputePipelineBuilder {
            public:
                explicit ComputePipelineBuilder(VkDevice device)
                    : device(device) {
                }

                ComputePipelineBuilder& SetShaderStages(const std::vector<VkPipelineShaderStageCreateInfo>& stages) {
                    shaderStages = stages;
                    return *this;
                }

                ComputePipelineBuilder& SetLayout(VkPipelineLayout layout) {
                    pipelineLayout = layout;
                    return *this;
                }

                Fox::Graphics::Vulkan::ComputePipelineBuilder& WithShader(Fox::Graphics::Enums::PipelineShaderStage stage, const std::string& shaderPath);


                void DestroyShaderModules();

                static VkShaderStageFlagBits ConvertShaderStage(Fox::Graphics::Enums::PipelineShaderStage stage);

                Fox::Graphics::Vulkan::Pipeline Build() {
                    VkComputePipelineCreateInfo info{};
                    info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
                    info.layout = pipelineLayout;
                    info.flags = 0;
                    info.stage = shaderStages[0];
                    info.pNext = nullptr;

                    VkPipeline pipeline;
                    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline) != VK_SUCCESS)
                        throw std::runtime_error("Failed to create graphics pipeline!");

                    DestroyShaderModules();

                    return Fox::Graphics::Vulkan::Pipeline(device, pipelineLayout, pipeline);
                }

            private:
                VkDevice device;
                VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
                std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
                std::vector<VkShaderModuleCreateInfo> shaderModuleCreateInfos;
                std::vector<VkShaderModule> shaderModules;
            };
        }
    }
}