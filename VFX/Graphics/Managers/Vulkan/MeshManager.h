#pragma once

#include <unordered_map>
#include "VFX/Graphics/Managers/Vulkan/MeshManager.h"

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				enum class MeshResource : int32_t {
					NONE = 0,
					SUBDIVIDED_PLANE = 1,
					CUBE = 2,
					DOUBLESIZEDCUBE = 3,
					UNIT_QUAD
				};

				class MeshManager : public Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::MeshManager> {  
					friend class Singleton <Fox::Graphics::Managers::Vulkan::MeshManager>;

				public: 

					MeshManager() = default;
					~MeshManager() = default;

					bool Initialize(VkDevice device);

					void Destroy() {
						meshes.clear();
					}

					inline std::unique_ptr<Fox::Graphics::Geometry::Vulkan::Mesh>& GetMesh(Fox::Graphics::Managers::Vulkan::MeshResource id) {
						return meshes[id];
					}

				private:
					std::unordered_map<Fox::Graphics::Managers::Vulkan::MeshResource, std::unique_ptr<Fox::Graphics::Geometry::Vulkan::Mesh>> meshes;
				};
			}
		}
	}
}