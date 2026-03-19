#pragma once

#include <glm/glm.hpp>

#include "VFX/Graphics/Geometry/Material.h"

namespace Fox {

	namespace Graphics {
	
		namespace Geometry {

			struct MaterialInstance {
				MaterialInstance() = default;
				virtual ~MaterialInstance() = default;
				virtual void* Copy() {
					return {};
				};
			};

			struct AlbedoMaterialInstance : public Fox::Graphics::Geometry::MaterialInstance {
				AlbedoMaterialInstance() = default;
				virtual ~AlbedoMaterialInstance() = default;
				virtual void* Copy() override {
					Fox::Geometry::AlbedoMaterial* material = new Fox::Geometry::AlbedoMaterial{};
					Fox::Graphics::Geometry::AlbedoMaterialInstance* mat = dynamic_cast<Fox::Graphics::Geometry::AlbedoMaterialInstance*>(this);
					material->albedo = mat->albedoMaterial.albedo;
					material->albedoIndex = mat->albedoMaterial.albedoIndex;

					return material;
				}
				Fox::Geometry::AlbedoMaterial albedoMaterial;
			};

			struct SpriteMaterialInstance : public Fox::Graphics::Geometry::MaterialInstance {
				SpriteMaterialInstance() = default;
				virtual ~SpriteMaterialInstance() = default;
				virtual void* Copy() override {
					Fox::Geometry::SpriteMaterial* instance = new Fox::Geometry::SpriteMaterial{};
					
					Fox::Graphics::Geometry::SpriteMaterialInstance* mat = dynamic_cast<Fox::Graphics::Geometry::SpriteMaterialInstance*>(this);
					instance->albedo = mat->spriteMaterial.albedo;
					instance->albedoIndex = mat->spriteMaterial.albedoIndex;
					//	instance->tintColor = mat-> spriteMaterial.tintColor; // TODO add tint color
					instance->maskTextureIndex = mat->spriteMaterial.maskTextureIndex;

					return instance;
				}
				Fox::Geometry::SpriteMaterial spriteMaterial;
			};
			struct PhysicallyBasedMaterialInstance : public Fox::Graphics::Geometry::MaterialInstance, public Fox::Geometry::PhysicallyBasedMaterial {};



			

		}
	
	}

}