#pragma once

#include "VFX/Scene/Scene.h"

#include <unordered_map>

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				enum class SceneId : int32_t {
					MAIN_SCENE = 0,
					MAIN_2D_SCENE,
					LAYER1_SCENE,
					LAYER2_SCENE,
					LAYER3_SCENE,
					LAYER4_SCENE,
					LAYER5_SCENE,
				};

				class SceneManager : public Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::SceneManager> {  
					friend class Singleton <Fox::Graphics::Managers::Vulkan::SceneManager>;

				public: 

					SceneManager() = default;
					~SceneManager() {
					}

					bool Initialize(VkDevice device, VkPhysicalDevice physicalDevice, std::unique_ptr<Fox::Graphics::Vulkan::CommandPool>& commandPool, VkQueue queue, VkSurfaceCapabilitiesKHR capabiliies);

					void Destroy() {
						scenes.clear();
						activeScenes.clear();
						commandPool = nullptr;
					}

					inline std::shared_ptr<Fox::Scene::Scene>& GetScene(Fox::Graphics::Managers::Vulkan::SceneId id) {
						return scenes[id]; 
					}

					inline std::shared_ptr<Fox::Scene::Scene>& GetCurrentScene() {
						return scenes[currentSceneId];
					}


					inline std::vector<std::shared_ptr<Fox::Scene::Scene>>& GetActiveScenes() {
						return activeScenes;
					}

					inline void ActivateScene(Fox::Graphics::Managers::Vulkan::SceneId sceneId, bool activate) {
						auto iter = std::find_if(activeScenes.begin(), activeScenes.end(), [=](std::shared_ptr <Fox::Scene::Scene>& scene) {
							return scene->GetID() == sceneId;
						});

						if (iter != activeScenes.end()) {
							if (!activate) {
								activeScenes.erase(iter);
							}
						}
						else {
							if (activate) {
								activeScenes.push_back(GetScene(sceneId));
							}
						}
					}

				private:
					std::unique_ptr<Fox::Graphics::Vulkan::CommandPool> commandPool;
					std::vector<std::shared_ptr<Fox::Scene::Scene>> activeScenes;

					SceneId currentSceneId = SceneId::MAIN_SCENE;
					std::unordered_map<Fox::Graphics::Managers::Vulkan::SceneId, std::shared_ptr<Fox::Scene::Scene>> scenes;
				};
			}
		}
	}
}