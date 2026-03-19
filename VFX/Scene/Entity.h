#pragma once

#include <vector>
#include <memory>

#include "VFX/Scene/Component.h"
#include "VFX/Scene/Transform3DComponent.h"

namespace Fox {
    namespace Scene {

        class Component;

        class World;

        template<typename R, typename N>
        class Hierarchy;

        class Entity {
        public:
            Entity() {
                Fox::Scene::Transform3DComponent* component = AddComponent<Fox::Scene::Transform3DComponent>();
                transform = component;
            };

            virtual ~Entity() = default;

            template<class ComponentType>
            ComponentType* AddComponent() {
                auto comp = std::make_shared<ComponentType>(this);
                components.push_back(std::move(comp));
                return dynamic_cast<ComponentType*>(components.back().get());
            }

            template<class ComponentType>
            ComponentType* SetupTransform() {
                auto comp = std::make_shared<ComponentType>(this);
                components.push_back(std::move(comp));
                transform = dynamic_cast<Fox::Scene::TransformComponent*>(components.back().get());
                return dynamic_cast<ComponentType*>(transform);
            }

            template<class ComponentType>
            ComponentType* FindComponent() const {
                for (auto& component : components) {
                    if (auto* c = dynamic_cast<ComponentType*>(component.get())) {
                        return c;
                    }
                }

                return nullptr;
            }

            Fox::Scene::TransformComponent* transform;
            std::vector<std::shared_ptr<Component>> components;
        };

        using SceneGraph = Hierarchy<World, Entity>;


    } 
}