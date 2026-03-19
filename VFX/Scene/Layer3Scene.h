#pragma once

#include <vector>
#include <memory>

namespace Fox {

    namespace Scene {

        class Layer3Scene : public Fox::Scene::Scene2D
        {
        public:
            Layer3Scene() = default;
            virtual ~Layer3Scene() {
            }


            virtual void Initialize(Fox::Graphics::Managers::Vulkan::SceneId id, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, VkSurfaceCapabilitiesKHR capabilities) override {
                this->device = device;
                this->physicalDevice = physicalDevice;
                this->commandPool = commandPool;
                this->queue = queue;
                this->capabilities = capabilities;
                Fox::Scene::Scene2D::Initialize(id, device, physicalDevice, commandPool, queue, capabilities);
                Fox::Scene::Scene2D::SetUpCamera(capabilities);

                auto root = sceneGraph->GetRoot();
                AddSprite("background", root,
                    glm::vec2(capabilities.currentExtent.width * 0.5f, capabilities.currentExtent.height * 0.5f), Fox::Scene::Main2DScene::imageDimensionPercentage * 3840.0f, Fox::Scene::Main2DScene::imageDimensionPercentage * 2160.0f, glm::vec2(Fox::Scene::Main2DScene::imageDimensionPercentage * 0.5f * 3840.0f, Fox::Scene::Main2DScene::imageDimensionPercentage * 0.5f * 2160.0f), 0.0f, 0);

                AddSprite("kettu1", root,
                    glm::vec2(capabilities.currentExtent.width * 0.3f, capabilities.currentExtent.height * 0.5f), Fox::Scene::Main2DScene::imageDimensionPercentage * 500.0f, Fox::Scene::Main2DScene::imageDimensionPercentage * 500.0f, glm::vec2(Fox::Scene::Main2DScene::imageDimensionPercentage * 0.5f * 500.0f, Fox::Scene::Main2DScene::imageDimensionPercentage * 0.5f * 500.0f), 0.0f, 1);

                AddSprite("kettu2", root,
                    glm::vec2(capabilities.currentExtent.width * 0.8f, capabilities.currentExtent.height * 0.7f), Fox::Scene::Main2DScene::imageDimensionPercentage * 500.0f, Fox::Scene::Main2DScene::imageDimensionPercentage * 500.0f, glm::vec2(Fox::Scene::Main2DScene::imageDimensionPercentage * 0.5f * 500.0f, Fox::Scene::Main2DScene::imageDimensionPercentage * 0.5f * 500.0f), 0.0f, 1);

                AddSprite("kettu3", root,
                    glm::vec2(capabilities.currentExtent.width * 0.8f, capabilities.currentExtent.height * 0.8f), Fox::Scene::Main2DScene::imageDimensionPercentage * 500.0f, Fox::Scene::Main2DScene::imageDimensionPercentage * 500.0f, glm::vec2(Fox::Scene::Main2DScene::imageDimensionPercentage * 0.5f * 500.0f, Fox::Scene::Main2DScene::imageDimensionPercentage * 0.5f * 500.0f), 0.0f, 1);

                BuildBuffers(device, physicalDevice, commandPool, queue);
            }

            virtual void Update(float deltaTime) {
                Fox::Scene::Scene2D::Update(deltaTime);
            }

            void AddVideoSprite(Fox::Graphics::Managers::Vulkan::VulkanVideoPlayerData& data, uint32_t materialIndex) {
                auto root = sceneGraph->GetRoot();
                AddSprite(data.name, root, glm::vec2(data.position), data.width, data.height, glm::vec2(data.width * 0.5f, 0.5f * data.height), 0.0f, materialIndex, data.renderOrder);
            }

        protected:
            VkDevice device;
            VkPhysicalDevice physicalDevice;
            VkCommandPool commandPool;
            VkQueue queue;
            VkSurfaceCapabilitiesKHR capabilities;
        };
    }
}