#pragma once

#include <glm/glm.hpp>

namespace Fox {

	namespace Geometry {

		struct Material {
			Material() = default;
			virtual ~Material() {
			};

			void Destroy() {
				delete material;
			}
			void* material;
		};

		struct alignas(16) AlbedoMaterial {
			glm::vec4 albedo;
			uint32_t albedoIndex;   //index into bindless array
			uint32_t _pad0;        // 4 bytes padding
			uint32_t _pad1;        // 4 bytes padding
			uint32_t _pad2;        // 4 bytes padding
		
		};

		struct SpriteMaterial {
			glm::vec4 albedo;
			uint32_t albedoIndex;   //index into bindless array
			uint32_t maskTextureIndex;        // 4 bytes padding
			uint32_t _pad1;        // 4 bytes padding
			uint32_t _pad2;        // 4 bytes padding
		//	glm::vec4 tintColor;
		//	uint32_t _pad3;        // 4 bytes padding
		//	uint32_t _pad4;        // 4 bytes padding
		//	uint32_t _pad5;        // 4 bytes padding
		//	uint32_t _pad6;
		};

		struct PhysicallyBasedMaterial {
			float metallic;
			float roughness;
			uint32_t metallicIndex;
			uint32_t roughnessIndex;
			uint32_t normalMapIndex;
		};

	}

}