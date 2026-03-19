#pragma once

#include "VFX/Scene/MeshComponent.h"

namespace Fox {
    namespace Graphics {
        namespace Managers {
            namespace Vulkan {
                enum class SceneId;
            }
        }
    }
}

namespace Fox {

    namespace Scene {

        class Scene
        {
        public:
            Scene() = default;
            virtual ~Scene() {
                vertexBuffer = nullptr;
                indexBuffer = nullptr;
                sceneGraph = nullptr;
            }

            virtual void Initialize(Fox::Graphics::Managers::Vulkan::SceneId id, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, VkSurfaceCapabilitiesKHR capabilities) {
                this->id = id;
                InitWorld();
            };

            virtual void Update(float deltaTime) = 0;
            virtual void InitWorld();

            virtual SceneGraph::Node* AddMeshToScene(const std::string& name, Fox::Graphics::Managers::Vulkan::MeshResource id, std::shared_ptr<Fox::Scene::SceneGraph::Node>& parent, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
                auto it = std::find_if(loadedMeshes.begin(), loadedMeshes.end(), [=](const Fox::Graphics::Managers::Vulkan::MeshResource resourceId) {
                    return resourceId == id;
                    });

				std::vector<Fox::Graphics::Vulkan::MeshInfo> meshInfos;

                if (it != loadedMeshes.end()) {
                    meshInfos = Fox::Graphics::Managers::Vulkan::MeshManager::Get().GetMesh(id)->GetMeshInfos();

                    for (auto& meshInfo : meshInfos) {
                        meshInfo.modelIndex = modelIndex;
                    }

                    sceneMeshInfos.insert(sceneMeshInfos.end(), meshInfos.begin(), meshInfos.end());
                }
                else {
                    loadedMeshes.push_back(id);

                    auto vertices = Fox::Graphics::Managers::Vulkan::MeshManager::Get().GetMesh(id)->GetVertices();
                    sceneVertices.insert(sceneVertices.end(), vertices.begin(), vertices.end());

                    auto indices = Fox::Graphics::Managers::Vulkan::MeshManager::Get().GetMesh(id)->GetIndices();
                    sceneIndices.insert(sceneIndices.end(), indices.begin(), indices.end());

                    meshInfos = Fox::Graphics::Managers::Vulkan::MeshManager::Get().GetMesh(id)->GetMeshInfos();

                    for (auto& meshInfo : meshInfos) {
                        meshInfo.vertexOffset += (sceneVertices.size() - vertices.size());
                        meshInfo.indexOffset += (sceneIndices.size() - indices.size());
                        meshInfo.modelIndex = modelIndex;
                    }

                    sceneMeshInfos.insert(sceneMeshInfos.end(), meshInfos.begin(), meshInfos.end());
                }

                std::shared_ptr<Fox::Scene::SceneGraph::Node> node = sceneGraph->AddChild(parent, name, std::make_shared<Fox::Scene::Entity>(), 0);
                auto* meshComponent = node->GetEntity<Fox::Scene::Entity>()->AddComponent<Fox::Scene::MeshComponent>();
                meshComponent->SetMesh(id, modelIndex, meshInfos);
                auto* transform = node->GetEntity<Fox::Scene::Entity>()->transform;

                transform->SetPosition(position);
                transform->SetRotation(rotation);
                transform->SetScale(scale);

                modelIndex++;

                return node.get();
            }

            const std::shared_ptr<Fox::Graphics::Vulkan::Camera>& GetMainCamera() const {
                return mainCamera;
            }

            inline std::unique_ptr<Fox::Graphics::Vulkan::VertexBuffer>& GetVertexBuffer() {
                return vertexBuffer;
            }

            inline std::unique_ptr<Fox::Graphics::Vulkan::IndexBuffer>& GetIndexBuffer() {
                return indexBuffer;
            }

            inline std::vector<Fox::Graphics::Vulkan::MeshInfo>& GetSceneMeshInfos() {
                return sceneMeshInfos;
            }

            inline std::unique_ptr<Fox::Scene::SceneGraph>& GetSceneGraph() {
                return sceneGraph;
            }

            Fox::Graphics::Managers::Vulkan::SceneId GetID() const {
                return id;
            }

            virtual bool Is3DScene() {
                return is3D;
            }

        protected:

            virtual void SetUpCamera(VkSurfaceCapabilitiesKHR capabilities) = 0;

            void BuildBuffers(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue) {

                if(sceneVertices.empty() || sceneIndices.empty())
					return;

                vertexBuffer = std::make_unique<Fox::Graphics::Vulkan::VertexBuffer>();
                vertexBuffer->Create(device, physicalDevice, commandPool, queue, sceneVertices);

                indexBuffer = std::make_unique<Fox::Graphics::Vulkan::IndexBuffer>();
                indexBuffer->Create(device, physicalDevice, commandPool, queue, sceneIndices);
            }

            Fox::Graphics::Managers::Vulkan::SceneId id;

            std::shared_ptr<Fox::Graphics::Vulkan::Camera> mainCamera;

            std::unique_ptr<Fox::Scene::SceneGraph> sceneGraph;

            // rendering, for now use only one vertex batch buffer
            std::vector<Fox::Graphics::Vulkan::MeshInfo> sceneMeshInfos;
            std::unique_ptr<Fox::Graphics::Vulkan::VertexBuffer> vertexBuffer;
            std::unique_ptr<Fox::Graphics::Vulkan::IndexBuffer> indexBuffer;

            std::vector<Fox::Graphics::Vulkan::Vertex> sceneVertices;
            std::vector<uint32_t> sceneIndices;

            uint32_t modelIndex = 0u;

            std::vector<Fox::Graphics::Managers::Vulkan::MeshResource> loadedMeshes;

            bool is3D = true;

        };
    }
}