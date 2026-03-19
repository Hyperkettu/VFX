#pragma once

#include <glm/glm.hpp>

#include "VFX/Scene/Component.h"

namespace Fox {

    namespace Scene {

        class PlaneOrientationControllerComponent : public Fox::Scene::Component {
        public:
            PlaneOrientationControllerComponent()
                : position(glm::vec3(-2.0f, 1.0f, 0.0f))
            {
            };

            PlaneOrientationControllerComponent(Fox::Scene::Entity* owner) :
                Fox::Scene::Component(owner), position(glm::vec3(-2.0, 1.0f, 0.0f)) {
            }

            ~PlaneOrientationControllerComponent() = default;

            virtual void Update(float deltaTime) override {

                if (position.x > 2.0f) {
                    position += glm::vec3(0.0f, -1.0f, 0.0f) * deltaTime;
                }
                else {
                    position += glm::vec3(1.0f, 0.0f, 0.0f) * deltaTime;
                }

                    owner->transform->SetPosition(position);
            }
        private:

            glm::vec3 position = glm::vec3(-2.0f, 1.0f, 0.0f);
        };
    }
}