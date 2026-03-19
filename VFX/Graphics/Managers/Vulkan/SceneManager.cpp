#include "FoxRenderer.h"

// Static member definitions
std::unique_ptr<Fox::Graphics::Managers::Vulkan::SceneManager> Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::SceneManager>::instance = nullptr;
std::once_flag Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::SceneManager>::initFlag;

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				bool SceneManager::Initialize(VkDevice device, VkPhysicalDevice physicalDevice, std::unique_ptr<Fox::Graphics::Vulkan::CommandPool>& commandPool, VkQueue queue, VkSurfaceCapabilitiesKHR capabilities) {
					this->commandPool = std::move(commandPool);
					auto scene = std::make_shared<Fox::Scene::MainScene>();
					scene->Initialize(Fox::Graphics::Managers::Vulkan::SceneId::MAIN_SCENE, device, physicalDevice, this->commandPool->Get(), queue, capabilities);
					scenes[Fox::Graphics::Managers::Vulkan::SceneId::MAIN_SCENE] = std::move(scene);

					ActivateScene(Fox::Graphics::Managers::Vulkan::SceneId::MAIN_SCENE, true);
					
					auto scene2D = std::make_shared<Fox::Scene::Main2DScene>();
					scene2D->Initialize(Fox::Graphics::Managers::Vulkan::SceneId::MAIN_2D_SCENE, device, physicalDevice, this->commandPool->Get(), queue, capabilities);
					scenes[Fox::Graphics::Managers::Vulkan::SceneId::MAIN_2D_SCENE] = std::move(scene2D);

		
					//ActivateScene(Fox::Graphics::Managers::Vulkan::SceneId::MAIN_2D_SCENE, true);

					return true;
				}

			}
		}
	}
}