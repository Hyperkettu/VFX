#pragma once

#include <vector>
#include <memory>

namespace Fox {

    namespace Scene {

        class Main2DScene: public Fox::Scene::Scene2D
        {
        public:
            Main2DScene() = default;  
            virtual ~Main2DScene() { 
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
                    glm::vec2(450.0f, capabilities.currentExtent.height * 0.5f), 100.0f, 2160.0f, glm::vec2(imageDimensionPercentage * 0.5f * 100.0f, imageDimensionPercentage * 0.5f * 2160.0f), 0.0f, 0);

                BuildBuffers(device, physicalDevice, commandPool, queue);
            }

            virtual void Update(float deltaTime) {
                Fox::Scene::Scene2D::Update(deltaTime);
            }

            static float imageDimensionPercentage;

        protected:
            VkDevice device;
			VkPhysicalDevice physicalDevice;
			VkCommandPool commandPool;
			VkQueue queue;
			VkSurfaceCapabilitiesKHR capabilities;
        };
    }
}