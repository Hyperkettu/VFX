#pragma once

namespace Fox {

    namespace Scene {

        class Entity;

        class Component {
        public:
            Component() {}
            Component(Fox::Scene::Entity* owner)
                : owner(owner)
            {
            };
            virtual ~Component() = default;

            virtual void Update(float deltaTime) = 0;

            Fox::Scene::Entity* GetOwner() {
                return owner;
            }
        protected:
            Entity* owner = nullptr;
        };
    }
}