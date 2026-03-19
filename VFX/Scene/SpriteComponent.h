#pragma once

#include "VFX/Scene/Entity.h"
#include "VFX/Graphics/Managers/Vulkan/MeshManager.h"

namespace Fox {

    namespace Scene {


        class SpriteComponent : public Fox::Scene::Component {
        public:
            SpriteComponent() = default;
            SpriteComponent(Fox::Scene::Entity* owner)
                : Fox::Scene::Component(owner), materialInstanceIndex(~0u)
            {
            }
            ~SpriteComponent() = default;

            void SetMaterial(uint32_t materialInstanceIndex) {
                this->materialInstanceIndex = materialInstanceIndex;
            }

            bool HasMaterial() const {
                return materialInstanceIndex != ~0u;
            }

            void SetRenderOrder(uint32_t newRenderOrder) {
                renderOrder = newRenderOrder;
                isDirty = true;
            }

            void MarkUndirty() {
                isDirty = false;
            }

            virtual void Update(float deltaTime) override {}

            uint32_t materialInstanceIndex = ~0u;

            glm::vec4 tintColor = glm::vec4(1.0f);
            uint32_t renderOrder = 0;

        private: 
            bool isDirty = true;
        };

    }
}