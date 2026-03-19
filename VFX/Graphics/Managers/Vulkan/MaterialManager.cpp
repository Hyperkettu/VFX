#include "FoxRenderer.h"
#include "VFX/Graphics/Geometry/MaterialDescription.h"

// Static member definitions
std::unique_ptr<Fox::Graphics::Managers::Vulkan::MaterialManager> Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::MaterialManager>::instance = nullptr;
std::once_flag Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::MaterialManager>::initFlag;

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				bool MaterialManager::Initialize(VkDevice device) {

					size_t size = static_cast<size_t>(Fox::Graphics::Managers::Vulkan::MaterialType::NUM_MATERIAL_TYPES);
					materialInstances.reserve(size);
					materialInstances.push_back({});
					materialInstances.push_back({});
					materialInstances.push_back({});

					indicesOfMaterialInstances.reserve(size);
					indicesOfMaterialInstances.push_back({});
					indicesOfMaterialInstances.push_back({});
					indicesOfMaterialInstances.push_back({});

					

					auto spriteDesc = Fox::Graphics::Geometry::SpriteMaterialDescription();
					spriteDesc.type = Fox::Graphics::Managers::Vulkan::MaterialType::SPRITE_MATERIAL;
					spriteDesc.albedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
					spriteDesc.albedoTexture = Fox::Graphics::Managers::Vulkan::TextureManager::Get().GetShaderResourceTexture("background").get();

					auto spriteMaterialIndex = RegisterMaterial(spriteDesc);

					auto spriteMaterial = GetMaterial(spriteMaterialIndex);
					auto spriteMaterialInstanceIndex = InstantianteMaterial(spriteMaterial, spriteDesc.type);
					(dynamic_cast<Fox::Graphics::Geometry::SpriteMaterialInstance*>(materialInstances[0][spriteMaterialInstanceIndex].get()))->spriteMaterial.albedo = glm::vec4(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f); 


					auto foxSpriteDesc = Fox::Graphics::Geometry::SpriteMaterialDescription();
					foxSpriteDesc.type = Fox::Graphics::Managers::Vulkan::MaterialType::SPRITE_MATERIAL;
					foxSpriteDesc.albedo = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
					foxSpriteDesc.albedoTexture = Fox::Graphics::Managers::Vulkan::TextureManager::Get().GetShaderResourceTexture("kettu").get();

					auto foxSpriteMaterialIndex = RegisterMaterial(foxSpriteDesc);

					auto foxSpriteMaterial = GetMaterial(foxSpriteMaterialIndex);
					auto foxSpriteMaterialInstanceIndex = InstantianteMaterial(foxSpriteMaterial, foxSpriteDesc.type);
					(dynamic_cast<Fox::Graphics::Geometry::SpriteMaterialInstance*>(materialInstances[0][foxSpriteMaterialInstanceIndex].get()))->spriteMaterial.albedo = glm::vec4(93.0f/255.0f, 231.0f/255.0f, 57.0f/255.0f, 1.0f); 

					return true;
				}

				void MaterialManager::BuildMaterials() {
				
				}

				uint32_t MaterialManager::AllocateTextureToBindlessArray(Fox::Graphics::Vulkan::Texture* tex) {
					auto iter = indicesOfTextures.find(tex);

					if (iter != indicesOfTextures.end()) {
						return iter->second;
					}

					bindlessTextures.push_back(tex);
					indicesOfTextures[tex] = static_cast<uint32_t>(bindlessTextures.size() - 1);

					return indicesOfTextures[tex];
				}

				uint32_t MaterialManager::RegisterMaterial(Fox::Graphics::Geometry::MaterialDescription& desc) {
					Fox::Graphics::Geometry::MaterialDescription* materialDescription = &desc;

					auto iter = std::find_if(
						registeredDescriptionsToMaterialIndex.begin(),
						registeredDescriptionsToMaterialIndex.end(),
						[materialDescription](const std::pair<Fox::Graphics::Geometry::MaterialDescription*, uint32_t>& current) {
							return current.first == materialDescription;
						});

					if (iter != registeredDescriptionsToMaterialIndex.end()) {
						return iter->second;
					}

					switch (desc.type)
					{
					case Fox::Graphics::Managers::Vulkan::MaterialType::SPRITE_MATERIAL: {
						Fox::Graphics::Geometry::SpriteMaterialDescription* description = dynamic_cast<Fox::Graphics::Geometry::SpriteMaterialDescription*>(&desc);
						Fox::Geometry::Material material{};
						material.material = new Fox::Geometry::SpriteMaterial{};
						auto mat = static_cast<Fox::Geometry::SpriteMaterial*>(material.material);
						mat->albedo = glm::vec4(1.0f); // default
						// mat->tintColor = glm::vec4(1.0f); // TODO add tint color to material description and set it here
						mat->albedoIndex = AllocateTextureToBindlessArray(description->albedoTexture);
						mat->maskTextureIndex = description->maskTexture ? AllocateTextureToBindlessArray(description->maskTexture) : ~0u;

						materials.push_back(std::make_shared<Fox::Geometry::Material>(material));
						break;
					}
					case Fox::Graphics::Managers::Vulkan::MaterialType::ALBEDO_MATERIAL: {
						Fox::Graphics::Geometry::AlbedoMaterialDescription* description = dynamic_cast<Fox::Graphics::Geometry::AlbedoMaterialDescription*>(&desc);
						auto* mat = new Fox::Geometry::AlbedoMaterial{};
						Fox::Geometry::Material material{};
						material.material = mat;
						mat->albedo = glm::vec4(1.0f);
						mat->albedoIndex = AllocateTextureToBindlessArray(description->albedoTexture);
						materials.push_back(std::make_shared<Fox::Geometry::Material>(material));	
						break;
					}
					case Fox::Graphics::Managers::Vulkan::MaterialType::PHYSICALLY_BASED_MATERIAL: {
						Fox::Graphics::Geometry::PhysicallyBasedMaterialDescription* description = dynamic_cast<Fox::Graphics::Geometry::PhysicallyBasedMaterialDescription*>(&desc);
						auto* mat = new Fox::Geometry::PhysicallyBasedMaterial{};
						Fox::Geometry::Material material{};
						//mat->albedo = glm::vec4(1.0f);
						//mat->albedoIndex = AllocateTextureToBindlessArray(description->albedoTexture);
						mat->metallic = 0.0f;
						mat->metallicIndex = AllocateTextureToBindlessArray(description->metallicTexture);
						mat->roughness = 0.0f;
						mat->roughnessIndex = AllocateTextureToBindlessArray(description->roughnessTexture);
						materials.push_back(std::make_shared<Fox::Geometry::Material>(material));
						break;

					}
					default:
						return ~0u;
					}

					indicesOfMaterials[materials.back().get()] = (uint32_t)materials.size() - 1;
					registeredDescriptionsToMaterialIndex[materialDescription] = indicesOfMaterials[materials.back().get()];

					return (uint32_t) materials.size() - 1;
				}

				uint32_t MaterialManager::InstantianteMaterial(Fox::Geometry::Material* material, Fox::Graphics::Managers::Vulkan::MaterialType materialType) {
					std::shared_ptr<Fox::Graphics::Geometry::MaterialInstance> instance;

					switch (materialType)	
					{
					case Fox::Graphics::Managers::Vulkan::MaterialType::SPRITE_MATERIAL: {
						instance = std::make_shared<Fox::Graphics::Geometry::SpriteMaterialInstance>();
						auto spriteMat = dynamic_cast<Fox::Graphics::Geometry::SpriteMaterialInstance*>(instance.get());
						auto spriteMaterial = static_cast<Fox::Geometry::SpriteMaterial*>(material->material);

						spriteMat->spriteMaterial.albedo = spriteMaterial->albedo;
						spriteMat->spriteMaterial.albedoIndex = spriteMaterial->albedoIndex;
						spriteMat->spriteMaterial.maskTextureIndex = spriteMaterial->maskTextureIndex;
					//	spriteMat->spriteMaterial.tintColor = spriteMaterial->tintColor; // TODO add tint color

						break;
					}
					case Fox::Graphics::Managers::Vulkan::MaterialType::ALBEDO_MATERIAL: {
						instance = std::make_shared<Fox::Graphics::Geometry::AlbedoMaterialInstance>();
						auto albedoMat = dynamic_cast<Fox::Graphics::Geometry::AlbedoMaterialInstance*>(instance.get());
						auto albedoMaterial = static_cast<Fox::Geometry::AlbedoMaterial*>(material->material);

						albedoMat->albedoMaterial.albedo = albedoMaterial->albedo;
						albedoMat->albedoMaterial.albedoIndex = albedoMaterial->albedoIndex;

						break;
					}					
					case Fox::Graphics::Managers::Vulkan::MaterialType::PHYSICALLY_BASED_MATERIAL: {
						instance = std::make_shared<Fox::Graphics::Geometry::PhysicallyBasedMaterialInstance>();
						auto pbrMat = dynamic_cast<Fox::Graphics::Geometry::PhysicallyBasedMaterialInstance*>(instance.get());
				//		auto pbrMaterial = dynamic_cast<Fox::Geometry::PhysicallyBasedMaterial*>(material);

				//		pbrMat->albedo = pbrMaterial->albedo;
				//		pbrMat->albedoIndex = pbrMaterial->albedoIndex;
				//		pbrMat->metallic = pbrMaterial->metallic;
				//		pbrMat->metallicIndex = pbrMaterial->metallicIndex;
				//		pbrMat->normalMapIndex = pbrMaterial->normalMapIndex;
				//		pbrMat->roughness = pbrMaterial->roughness;
				//		pbrMat->roughnessIndex = pbrMaterial->roughnessIndex;
					
						break;
					}
						
					default:
						return ~0u;
					}

					if (!instance.get()) {
						return ~0u;
					}

					materialInstances[static_cast<uint32_t>(materialType)].push_back(std::move(instance));
					auto index = materialInstances[static_cast<uint32_t>(materialType)].size() - 1;
					indicesOfMaterialInstances[static_cast<uint32_t>(materialType)][materialInstances[static_cast<uint32_t>(materialType)].back()] = index;
					return static_cast<uint32_t>(index);
				}



			}
		}
	}
}