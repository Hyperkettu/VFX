#pragma once

#include "VFX/Scene/Transform2DComponent.h"
#include "VFX/Scene/SpriteComponent.h"

namespace Fox {

    namespace Scene {

        class Scene2D : public Fox::Scene::Scene
        {
        public:
            Scene2D() = default;
            virtual ~Scene2D() {
            }

            virtual void Initialize(Fox::Graphics::Managers::Vulkan::SceneId id, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, VkSurfaceCapabilitiesKHR capabilities) override {
                Scene::Initialize(id, device, physicalDevice, commandPool, queue, capabilities);
            }

            Fox::Scene::SceneGraph::Node* AddSprite(const std::string& name, std::shared_ptr<Fox::Scene::SceneGraph::Node>& parent, const glm::vec2 position, float width, float height, const glm::vec2& pivot, float angle, uint32_t materialIndex, float renderOrder = 0.0f) {
                auto* node = AddMeshToScene(name, Fox::Graphics::Managers::Vulkan::MeshResource::UNIT_QUAD, parent, glm::vec3(position, 0.0f), glm::vec3(0.0f, 0.0f, angle), glm::vec3(width, height, 1.0f));
                auto* entity = node->GetEntity<Fox::Scene::Entity>();
                auto* transform = entity->SetupTransform<Fox::Scene2D::Transform2DComponent>();
                transform->SetPivot(pivot);
                transform->SetPosition(glm::vec3(position, 0.0f));
                transform->SetRotation(angle);
                transform->SetScale(glm::vec3(width, height, 1.0f));
                auto* spriteComponent = entity->AddComponent<Fox::Scene::SpriteComponent>();
                auto materialInstance = Fox::Graphics::Managers::Vulkan::MaterialManager::Get().GetMaterialInstance(Fox::Graphics::Managers::Vulkan::MaterialType::SPRITE_MATERIAL, materialIndex);
                spriteComponent->SetMaterial(materialIndex);
				spriteComponent->SetRenderOrder(renderOrder);
                return node;
            }

            virtual void Update(float deltaTime) override {
                sceneGraph->Update(deltaTime);
            };


            virtual bool Is3DScene() override {
                return false;
            }

        protected:

            virtual void SetUpCamera(VkSurfaceCapabilitiesKHR capabilities) override {
                mainCamera = std::make_shared<Fox::Graphics::Vulkan::Camera2D>(capabilities.currentExtent.width, capabilities.currentExtent.height);
            }
        };
    }
}