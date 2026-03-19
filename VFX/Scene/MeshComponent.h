#pragma once

#include "VFX/Scene/Entity.h"
#include "VFX/Graphics/Managers/Vulkan/MeshManager.h"

namespace Fox {

    namespace Scene {


        class MeshComponent : public Fox::Scene::Component {
        public:
            MeshComponent() = default;
            MeshComponent(Fox::Scene::Entity* owner)
                : Fox::Scene::Component(owner), meshId(Fox::Graphics::Managers::Vulkan::MeshResource::NONE), modelIndex(~0u), materialInstanceIndex(~0u)
            {
            }
            ~MeshComponent() = default;

            void SetMesh(Fox::Graphics::Managers::Vulkan::MeshResource meshId, uint32_t modelIndex, std::vector<Fox::Graphics::Vulkan::MeshInfo>& meshInfos) {
                this->meshId = meshId;
                this->modelIndex = modelIndex;
                this->meshInfos = meshInfos;
            }

            void SetMaterial(uint32_t material) {
                materialInstanceIndex = material;
            }

            bool HasMesh() const {
                return meshId != Fox::Graphics::Managers::Vulkan::MeshResource::NONE && modelIndex >= 0;
            }

            bool HasMaterial() const {
                return materialInstanceIndex != ~0u;
            }

            std::vector<Fox::Graphics::Vulkan::MeshInfo>& GetMeshInfos() {
                return meshInfos;
			}

            void SetDepth(float depth) {
                for (auto& meshInfo : meshInfos) {
                    meshInfo.depth = depth;
                }
			}

            virtual void Update(float deltaTime) override {}

            Fox::Graphics::Managers::Vulkan::MeshResource meshId;
            uint32_t modelIndex;
            uint32_t materialInstanceIndex;
            std::vector<Fox::Graphics::Vulkan::MeshInfo> meshInfos;
        };

    }
}