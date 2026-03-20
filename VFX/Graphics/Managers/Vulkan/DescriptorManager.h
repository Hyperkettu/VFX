#pragma once

#include <unordered_map>

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				enum class Descriptor : int32_t {
					MAIN_MESH_SHADER = 0,
					MAIN_MESH_BINDLESS_TEXTURING_SHADER,
					OFFSCREEN,
					POST_PROCESSOR,
					EULER_PARTICLE_UPDATE,
					EULER_PARTICLE_RENDER
				};

				class DescriptorManager : public Fox::Core::Singleton<DescriptorManager> {
					friend class Singleton <Fox::Graphics::Managers::Vulkan::DescriptorManager>;

				public: 

					DescriptorManager() = default;
					~DescriptorManager() = default;

					bool Initialize(VkDevice device, const Fox::Graphics::RendererConfig& config, uint32_t sceneCount);

					void Destroy() {
						descriptorSetBuilders.clear();
					}

					std::unique_ptr<Fox::Graphics::Vulkan::DescriptorSetBuilder>& GetDescriptorSet(Fox::Graphics::Managers::Vulkan::Descriptor set) {
						return descriptorSetBuilders[set];
					}

				private:
					std::unordered_map<Fox::Graphics::Managers::Vulkan::Descriptor, std::unique_ptr<Fox::Graphics::Vulkan::DescriptorSetBuilder>> descriptorSetBuilders;
				};
			}
		}
	}
}