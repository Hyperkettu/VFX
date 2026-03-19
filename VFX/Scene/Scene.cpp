#include "FoxRenderer.h"

namespace Fox {

    namespace Scene {

        void Scene::InitWorld() {
            sceneGraph = std::make_unique<Fox::Scene::SceneGraph>();
        }

    }
}