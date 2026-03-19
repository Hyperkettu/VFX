#pragma once

#include "VFX/Scene/Entity.h"
#include "VFX/Scene/TransformComponent.h"

#include <glm/glm.hpp>

namespace Fox {

    namespace Scene {

        using Vector = glm::vec3;
        using Matrix = glm::mat4;


        class Transform3DComponent : public Fox::Scene::TransformComponent {
        public:
            Transform3DComponent() = default;
            Transform3DComponent(Fox::Scene::Entity* owner)
                : Fox::Scene::TransformComponent(owner)
            {
            }
            virtual ~Transform3DComponent() {};

            virtual void Update(float deltaTime) override {

            }

            virtual void SetPosition(const Vector& newPos) override
            {
                if (position != newPos)
                {
                    position = newPos;
                    MarkDirty();
                }
            }

            virtual void SetRotation(const Vector& newRot) override
            {
                if (rotation != newRot)
                {
                    rotation = newRot;
                    MarkDirty();
                }
            }

            virtual void SetScale(const Vector& newScale) override
            {
                if (scale != newScale)
                {
                    scale = newScale;
                    MarkDirty();
                }
            }

            virtual void UpdateLocalMatrix() override
            {
                if (!isDirty)
                    return;

                Matrix T = glm::translate(Matrix(1.0f), position);

                Matrix R(1.0f);

                R = glm::rotate(R, rotation.x, Vector(1.0f, 0.0f, 0.0f)); // Pitch
                R = glm::rotate(R, rotation.y, Vector(0.0f, 1.0f, 0.0f)); // Yaw
                R = glm::rotate(R, rotation.z, Vector(0.0f, 0.0f, 1.0f)); // Roll

                Matrix S = glm::scale(Matrix(1.0f), scale);

                localMatrix = T * R * S;

                isDirty = false;
            }

        protected:
            Fox::Scene::Vector position;
            Fox::Scene::Vector scale;
            Fox::Scene::Vector rotation;

        };

    }
}