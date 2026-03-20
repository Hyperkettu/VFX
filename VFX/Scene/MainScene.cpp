#include "FoxRenderer.h"
#include "VFX/Scene/MeshComponent.h"
#include "VFX/Scene/Transform2DComponent.h"
#include "VFX/Scene/PlaneOrientationControllerComponent.h"

namespace Fox {

    namespace Scene {

        void MainScene::Initialize(Fox::Graphics::Managers::Vulkan::SceneId id, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, VkSurfaceCapabilitiesKHR capabilities) {
			Scene::Initialize(id, device, physicalDevice, commandPool, queue, capabilities);
			SetUpCamera(capabilities);

			auto root = sceneGraph->GetRoot();
			AddMesh("Cube", Fox::Graphics::Managers::Vulkan::MeshResource::CUBE, root, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f), 0);



			BuildBuffers(device, physicalDevice, commandPool, queue);
        }

        void MainScene::Update(float deltaTime) {
			sceneGraph->Update(deltaTime);
		}

		Fox::Scene::SceneGraph::Node* MainScene::AddMesh(const std::string& name, 
			Fox::Graphics::Managers::Vulkan::MeshResource meshResource, 
			std::shared_ptr<Fox::Scene::SceneGraph::Node>& parent, 
			const glm::vec3& position, const glm::vec3& rotation, 
			const glm::vec3& scale, 
			uint32_t materialIndex) {
			auto* node = AddMeshToScene(name, meshResource, parent, position, rotation, scale);
			auto* meshComponent = node->GetEntity<Fox::Scene::Entity>()->FindComponent<Fox::Scene::MeshComponent>();
			meshComponent->SetMaterial(materialIndex);

			return node;
		}

		void MainScene::SetUpCamera(VkSurfaceCapabilitiesKHR capabilities) {
			mainCamera = std::make_unique<Fox::Graphics::Vulkan::Camera3D>(45.0f, static_cast<float>(capabilities.currentExtent.width) / static_cast<float>(capabilities.currentExtent.height), 0.1f, 10000.0f);
			auto* camera = dynamic_cast<Fox::Graphics::Vulkan::Camera3D*>(mainCamera.get());
			camera->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
			camera->SetCameraTarget(glm::vec3(0.0f, 0.0f, 0.0f));
			camera->SetWorldUp(glm::vec3(0.0f, 1.0f, 0.0f));
		}
    }
}