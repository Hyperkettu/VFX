#pragma once

#include <unordered_map>

#include "VFX/Graphics/Geometry/Material.h"
#include "VFX/Graphics/Geometry/MaterialInstance.h"

namespace Fox {

	namespace Graphics {

		namespace Geometry {
			struct MaterialDescription;
		}

		namespace Managers {

			namespace Vulkan {

				enum class MaterialType : int32_t {
					SPRITE_MATERIAL = 0,
					ALBEDO_MATERIAL,
					PHYSICALLY_BASED_MATERIAL,
					NUM_MATERIAL_TYPES
				};

				class MaterialManager : public Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::MaterialManager> {   
					friend class Singleton<Fox::Graphics::Managers::Vulkan::MaterialManager>;

				public: 

					static const uint32_t NUM_TEXTURES_PER_BINDLESS_ARRAY = 32u; // 1024u

					MaterialManager() {
					};
					~MaterialManager() = default;

					bool Initialize(VkDevice device);

					void Destroy() {

						for (auto& material : materials) {
							material->Destroy();
						}

						materials.clear();
						materialInstances.clear();
						bindlessTextures.clear();
					}

					void BuildMaterials();

					inline std::shared_ptr<Fox::Graphics::Geometry::MaterialInstance>& GetMaterialInstance(Fox::Graphics::Managers::Vulkan::MaterialType materialType, uint32_t index) {
						return materialInstances[static_cast<uint32_t>(materialType)][index];
					}

					inline const std::vector<Fox::Graphics::Vulkan::Texture*>& GetBindlessTextureArray() const {
						return bindlessTextures;
					}

					inline std::vector<Fox::Graphics::Vulkan::Texture*>& GetBindlessTextureArray() {
						return bindlessTextures;
					}

					uint32_t AllocateTextureToBindlessArray(Fox::Graphics::Vulkan::Texture* tex);
					uint32_t RegisterMaterial(Fox::Graphics::Geometry::MaterialDescription& desc);
					uint32_t InstantianteMaterial(Fox::Geometry::Material* material, Fox::Graphics::Managers::Vulkan::MaterialType materialType);
					uint32_t GetIndexOfMaterialInstance(Fox::Graphics::Managers::Vulkan::MaterialType materialType, std::shared_ptr<Fox::Graphics::Geometry::MaterialInstance>& materialInstance) {
						std::cout << "iomi:" << indicesOfMaterialInstances[static_cast<uint32_t>(materialType)][materialInstance] << std::endl;
						return indicesOfMaterialInstances[static_cast<uint32_t>(materialType)][materialInstance];
					}

				
					Fox::Geometry::Material* GetMaterial(uint32_t index) {
						return materials[index].get();
					}

					uint32_t GetMaterialCount(Fox::Graphics::Managers::Vulkan::MaterialType materialType) const {
						return materialInstances[static_cast<uint32_t>(materialType)].size();
					}

				private:
					std::vector<std::shared_ptr<Fox::Geometry::Material>> materials; // by material type
					std::vector<std::vector<std::shared_ptr<Fox::Graphics::Geometry::MaterialInstance>>> materialInstances; // by material types
					std::vector<Fox::Graphics::Vulkan::Texture*> bindlessTextures; // by type?

					std::unordered_map<Fox::Graphics::Vulkan::Texture*, uint32_t> indicesOfTextures;
					std::unordered_map<Fox::Geometry::Material*, uint32_t> indicesOfMaterials;
					std::vector<std::unordered_map<std::shared_ptr<Fox::Graphics::Geometry::MaterialInstance>, uint32_t>> indicesOfMaterialInstances;
					std::unordered_map<Fox::Graphics::Geometry::MaterialDescription*, uint32_t> registeredDescriptionsToMaterialIndex;

				};
			}
		}
	}
}