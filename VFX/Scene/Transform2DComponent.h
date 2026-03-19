#pragma once

#include "VFX/Scene/Entity.h"
#include "VFX/Scene/TransformComponent.h"

#include <glm/glm.hpp>

namespace Fox {

    namespace Scene2D {

        using Vector = glm::vec3;
        using Matrix = glm::mat4;


        class Transform2DComponent : public Fox::Scene::TransformComponent {
        public:
            Transform2DComponent()
                : position(glm::vec2(0.0f)), scale(glm::vec2(1.0f)), pivot(glm::vec2(0.0f)), angle(0.0f)
            {};
            Transform2DComponent(Fox::Scene::Entity* owner)
                : Fox::Scene::TransformComponent(owner), position(glm::vec2(0.0f)), scale(glm::vec2(1.0f)), pivot(glm::vec2(0.0f)), angle(0.0f)
            {
            }
            virtual ~Transform2DComponent() {};

            virtual void Update(float deltaTime) override {
            
            }

            virtual void SetPosition(const Vector& newPos) override
            {
                if (position.x != newPos.x || position.y != newPos.y)
                {
                    position = glm::vec2(newPos);
                    MarkDirty();
                }
            }

            virtual void SetRotation(const Vector& newRotation) override {
                if (angle != newRotation.z)
                {
                    angle =  newRotation.z;
                    MarkDirty();
                }
            }

            void SetRotation(float newAngle)
            {
                if (angle != newAngle)
                {
                    angle = newAngle;
                    MarkDirty();
                }
            }

            virtual void SetScale(const Vector& newScale) override
            {
                if (scale.x != newScale.x || scale.y != newScale.y)
                {
                    scale = glm::vec2(newScale);
                    MarkDirty();
                }
            }

            void SetPivot(const glm::vec2& newPivot) {
                if (pivot != newPivot) {
                    pivot = newPivot;
                    isDirty = true;
                }
            }

            virtual void UpdateLocalMatrix() override
            {
                if (!isDirty)
                    return;

                glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
                glm::mat4 P = glm::translate(glm::mat4(1.0f), glm::vec3(-pivot, 0.0f));
                glm::mat4 R = glm::mat4(Rotate2D(angle));
                glm::mat4 S = glm::scale(glm::mat4(1.0f), {scale.x, scale.y, 1.0f});

                localMatrix = T * R * P * S;

                isDirty = false;
            }

        protected:

            glm::mat3 Rotate2D(float angle) {
                float c = glm::cos(angle);
                float s = glm::sin(angle);

                return glm::mat3(
                    c, -s, 0.0f,
                    s, c, 0.0f,
                    0.0f, 0.0f, 1.0f
                );
            }

            glm::vec2 position;
            glm::vec2 scale;
            glm::vec2 pivot;
            float angle;

            bool isDirty = true;

        };

    }
}