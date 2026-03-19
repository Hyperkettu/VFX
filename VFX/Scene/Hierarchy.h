#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "World.h"

namespace Fox {

    namespace Scene {

        template <class RootNode, class Entity>
        class Hierarchy {


        public:
            class Node;

            class Node {
                public:
                std::string name;
                std::shared_ptr<Entity> entity = nullptr;  // Attached scene node data (e.g., mesh, light)
                int layerID = 0;                  // Layer index (0 = opaque, 1 = transparent, etc.)
                bool isDirty = true;

                std::vector<std::shared_ptr<Node>> children;
                Node* parent = nullptr;

                Node() = default;

                Node(const std::string& name)
                    : name(name) {
                }
                
                template<class T>
                Node(const std::string& name, T&& entity, int32_t layerId = 0)
                    : name(name), entity(entity), layerID(layerId) {
                    
                }

                Node(const std::string& name, Node* parent, int32_t layerId = 0)
                    : name(name), parent(parent), layerID(layerId) {
                }

                template<class T>
                Node(const std::string& name, const std::shared_ptr<T>& entity, int32_t layerId = 0)
                    : name(name), entity(entity), layerID(layerId) {
                }


                template<class T> 
                T* GetEntity() {
                    return dynamic_cast<T*>(entity.get());
                }


            };
            Hierarchy()
            {
                root = std::make_shared<Node>("Root", std::make_shared<RootNode>());
                root.get()->entity->transform->SetPosition(glm::vec3(0.0f));
                root.get()->entity->transform->SetRotation(glm::vec3(0.0f));
                root.get()->entity->transform->SetScale(glm::vec3(1.0f));
            }

            ~Hierarchy() 
            {}

            std::shared_ptr<Node> GetRoot() const { return root; }

            std::shared_ptr<Node> AddChild(
                std::shared_ptr<Node> parent,
                const std::string& name,
                std::shared_ptr<Entity> entity = nullptr,
                int32_t layerID = 0)
            {
                auto newNode = std::make_shared<Node>(name, std::move(entity), layerID);
                newNode->parent = parent.get();
                parent->children.push_back(newNode);
                return newNode;
            }

            void Update(float deltaTime)
            {
                UpdateNode(root, Matrix(1.0f), deltaTime);
            }

            Node* GetByName(const std::string& name)
            {
                return FindNode(root.get(), name);
            }

            std::vector<std::shared_ptr<Node>> CollectByLayer(int32_t targetLayer)
            {
                std::vector<std::shared_ptr<Node>> collected;
                CollectByLayer(root, targetLayer, collected);
                return collected;
            }

            void RemoveByName(const std::string& name) {
                auto* node = FindNode(root.get(), name);
                if (node) {
                    RemoveNode(node);
                }
            }

        private:
            std::shared_ptr<Node> root;

            void UpdateNode(std::shared_ptr<Node> node, const Matrix& parentWorld, float deltaTime)
            {
                for (auto& component : node->entity->components) {
                    component->Update(deltaTime); 
                }

                Fox::Scene::Entity* entity = node->GetEntity<Fox::Scene::Entity>();

                entity->transform->UpdateLocalMatrix();
                entity->transform->worldMatrix = parentWorld * entity->transform->localMatrix;

                for (auto& child : node->children)
                    UpdateNode(child, entity->transform->worldMatrix, deltaTime);
            }

            Node* FindNode(Node* node, const std::string& name)
            {
                if (node->name == name)
                    return node;

                for (auto& child : node->children)
                {
                    if (auto* found = FindNode(child.get(), name))
                        return found;
                }

                return nullptr;
            }

            void RemoveNode(Node* node) {
                if (node->parent) {
                    auto& siblings = node->parent->children;
                    auto iter = std::remove_if(siblings.begin(), siblings.end(),
                        [node](const std::shared_ptr<Node>& n) { return n.get()->name == node->name; });
                    siblings.erase(iter, siblings.end());


                // siblings.erase(std::remove(siblings.begin(), siblings.end(), node), siblings.end());
                }
			}

            void CollectByLayer(std::shared_ptr<Node> node, int32_t targetLayer,
                std::vector<std::shared_ptr<Node>>& out)
            {
                if (node->layerID == targetLayer && node->entity) {
                    if (node.get() != root.get()) {
                        out.push_back(node);
                    }
                }

                for (auto& child : node->children)
                    CollectByLayer(child, targetLayer, out);
            }
        };

    }
}
