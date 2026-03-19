#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

namespace Fox {

    namespace Scene {

        class MainScene: public Fox::Scene::Scene/*3D*/
        {
        public:
            MainScene() = default;  
            virtual ~MainScene() {}


            virtual void Initialize(Fox::Graphics::Managers::Vulkan::SceneId id, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, VkSurfaceCapabilitiesKHR capabilities) override;
            virtual void Update(float deltaTime) override;
            virtual void SetUpCamera(VkSurfaceCapabilitiesKHR capabilities) override;

            Fox::Scene::SceneGraph::Node* AddMesh(const std::string& name, Fox::Graphics::Managers::Vulkan::MeshResource meshResource, std::shared_ptr<Fox::Scene::SceneGraph::Node>& parent, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, uint32_t materialIndex);

        protected:
        };
    }
}