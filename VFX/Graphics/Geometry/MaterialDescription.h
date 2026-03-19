#pragma once

#include <glm/glm.hpp>

#include "VFX/Graphics/Vulkan/Texture.h"
#include "VFX/Graphics/Managers/Vulkan/MaterialManager.h"

namespace Fox {

	namespace Graphics {

		namespace Geometry {

			struct MaterialDescription {
				MaterialDescription() {}
				virtual ~MaterialDescription() {};
				Fox::Graphics::Managers::Vulkan::MaterialType type;
			};

			struct AlbedoMaterialDescription : public Fox::Graphics::Geometry::MaterialDescription {
				glm::vec4 albedo;
				Fox::Graphics::Vulkan::Texture* albedoTexture;
			};

			struct SpriteMaterialDescription : public Fox::Graphics::Geometry::AlbedoMaterialDescription {
				glm::vec4 tintColor;
				Fox::Graphics::Vulkan::Texture* maskTexture = nullptr;
			};

			struct PhysicallyBasedMaterialDescription : public Fox::Graphics::Geometry::AlbedoMaterialDescription {
				float metallic;
				float roughness;
				Fox::Graphics::Vulkan::Texture* roughnessTexture;
				Fox::Graphics::Vulkan::Texture* metallicTexture;
				Fox::Graphics::Vulkan::Texture* normalTexture;
			}; 
		}
	}
}