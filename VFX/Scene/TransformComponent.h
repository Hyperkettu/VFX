#pragma once

#include "VFX/Scene/Entity.h"

#include <glm/glm.hpp>

namespace Fox {

    namespace Scene {

        using Vector = glm::vec3;
        using Matrix = glm::mat4;


        class TransformComponent : public Fox::Scene::Component {
        public:
            TransformComponent() = default;
            TransformComponent(Fox::Scene::Entity* owner)
                : Fox::Scene::Component(owner)
            {
            }
            virtual ~TransformComponent() {};

            virtual void Update(float deltaTime) override {
            
            }

            virtual void SetPosition(const Vector& newPos) = 0;

            template<class T>
            T* GetEntity() {
                return dynamic_cast<T*>(owner.get());
            }

            virtual void SetRotation(const Vector& newRot) = 0;
            virtual void SetScale(const Vector& newScale) = 0;

            void MarkDirty() {
                isDirty = true;
            }

            Fox::Scene::Matrix localMatrix{ 1.0f };
            Fox::Scene::Matrix worldMatrix{ 1.0f };

            virtual void UpdateLocalMatrix() = 0;

        protected:
            bool isDirty = true;
        };

    }
}