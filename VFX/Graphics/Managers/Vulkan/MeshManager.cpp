#include "FoxRenderer.h"

// Static member definitions
std::unique_ptr<Fox::Graphics::Managers::Vulkan::MeshManager> Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::MeshManager>::instance = nullptr;
std::once_flag Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::MeshManager>::initFlag;

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				bool MeshManager::Initialize(VkDevice device) {
					
					auto plane = std::make_unique<Fox::Graphics::Geometry::Vulkan::Mesh>(Fox::Graphics::Geometry::GeometryGenerator::GeneratePlaneMesh(16, 9, 1920 / 16.0f, 0.25f, 0.0f));
					meshes[Fox::Graphics::Managers::Vulkan::MeshResource::SUBDIVIDED_PLANE] = std::move(plane);

					auto cube = std::make_unique<Fox::Graphics::Geometry::Vulkan::Mesh>(Fox::Graphics::Geometry::GeometryGenerator::GenerateCubeMesh(1.0f));
					meshes[Fox::Graphics::Managers::Vulkan::MeshResource::CUBE] = std::move(cube);

					auto doubleCube = std::make_unique<Fox::Graphics::Geometry::Vulkan::Mesh>(Fox::Graphics::Geometry::GeometryGenerator::GenerateCubeMesh(2.0f));
					meshes[Fox::Graphics::Managers::Vulkan::MeshResource::DOUBLESIZEDCUBE] = std::move(doubleCube);

					auto unitQuad = std::make_unique<Fox::Graphics::Geometry::Vulkan::Mesh>(Fox::Graphics::Geometry::GeometryGenerator::GeneratePlaneMesh(1, 1, 1.0f, 1.0f, 0.0f));
					meshes[Fox::Graphics::Managers::Vulkan::MeshResource::UNIT_QUAD] = std::move(unitQuad);

					return true;
				}

			}
		}
	}
}