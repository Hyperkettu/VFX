#include "FoxRenderer.h"

#include "VFX/Graphics/Geometry/MaterialInstance.h"

// Static member definitions
std::unique_ptr<Fox::Graphics::Managers::Vulkan::FrameResourceManager> Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::FrameResourceManager>::instance = nullptr;
std::once_flag Fox::Core::Singleton<Fox::Graphics::Managers::Vulkan::FrameResourceManager>::initFlag;

namespace Fox {

	namespace Graphics {

		namespace Managers {

			namespace Vulkan {

				bool FrameResourceManager::Initialize(VkDevice device, const Fox::Graphics::RendererConfig& config, VkSurfaceCapabilitiesKHR capabilities, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkFormat depthFormat, int32_t graphicsQueueFamily) {
					this->device = device;

					frameResources.resize(config.MAX_FRAMES_IN_FLIGHT);
					for (auto i = 0u; i < config.MAX_FRAMES_IN_FLIGHT; i++)
					{
						frameResources[i] = std::make_unique<Fox::Graphics::Vulkan::FrameResource>();
						frameResources[i]->frameIndex = i;

						frameResources[i]->CreateGraphicsCommandResources(device, graphicsQueueFamily);

						if (!frameResources[i]->CreateSynchronizationObjects(device, true))
						{
							std::cerr << "Failed to create synchronization objects for a frame!\n";
							return 0;
						}
					}

					swapchain = std::make_unique<Fox::Graphics::Vulkan::Swapchain>(
						device,
						Fox::Graphics::Managers::Vulkan::RenderPassManager::Get().GetPass(Fox::Graphics::Managers::Vulkan::RenderPass::DEFAULT)->Get(),
						Fox::Graphics::Managers::Vulkan::TextureManager::Get().GetDepthTexture(Fox::Graphics::Managers::Vulkan::DepthTexture::DEFAULT_DEPTH_STENCIL)->GetView(),
						surface,
						surfaceFormat,
						capabilities,
						config.MAX_FRAMES_IN_FLIGHT);


					std::vector<VkImageView> attachments = {
						Fox::Graphics::Managers::Vulkan::TextureManager::Get().GetRenderTargetTexture(Fox::Graphics::Managers::Vulkan::RenderTargetTexture::DEFAULT_RENDER_TARGET)->GetView(),
						Fox::Graphics::Managers::Vulkan::TextureManager::Get().GetDepthTexture(Fox::Graphics::Managers::Vulkan::DepthTexture::DEFAULT_DEPTH_STENCIL)->GetView()
					};


					auto offscreenTarget = std::make_unique<Fox::Graphics::Vulkan::Framebuffer>(
						device,
						Fox::Graphics::Managers::Vulkan::RenderPassManager::Get().GetPass(Fox::Graphics::Managers::Vulkan::RenderPass::OFFSCREEN)->Get(),
						attachments,
						capabilities.currentExtent.width,
						capabilities.currentExtent.height
					);

					frameBuffers[Fox::Graphics::Managers::Vulkan::FrameBuffer::OFFSCREEN_TARGET] = std::move(offscreenTarget);

					for(auto i = 0u; i < 5; i++) {

						std::vector<VkImageView> attachmentsLayer = {
							Fox::Graphics::Managers::Vulkan::TextureManager::Get().GetRenderTargetTexture(static_cast<Fox::Graphics::Managers::Vulkan::RenderTargetTexture>(static_cast<int32_t>(Fox::Graphics::Managers::Vulkan::RenderTargetTexture::LAYER1_RENDER_TARGET) + i))->GetView(),
							Fox::Graphics::Managers::Vulkan::TextureManager::Get().GetDepthTexture(Fox::Graphics::Managers::Vulkan::DepthTexture::DEFAULT_DEPTH_STENCIL)->GetView()
						};

						auto offscreenTargetLayer = std::make_unique<Fox::Graphics::Vulkan::Framebuffer>(
							device,
							Fox::Graphics::Managers::Vulkan::RenderPassManager::Get().GetPass(Fox::Graphics::Managers::Vulkan::RenderPass::OFFSCREEN)->Get(),
							attachmentsLayer,
							capabilities.currentExtent.width,
							capabilities.currentExtent.height
						);
						frameBuffers[static_cast<Fox::Graphics::Managers::Vulkan::FrameBuffer>(static_cast<int32_t>(Fox::Graphics::Managers::Vulkan::FrameBuffer::OFFSCREEN_TARGET_LAYER1) + i)] = std::move(offscreenTargetLayer);
					}

					return true;
				}

				int32_t FrameResourceManager::CreateUniformBuffers(const Fox::Graphics::RendererConfig& config, VkPhysicalDevice physicalDevice) {
					auto& scenes = Fox::Graphics::Managers::Vulkan::SceneManager::Get().GetActiveScenes();

					for (uint32_t i = 0; i < config.MAX_FRAMES_IN_FLIGHT; i++) {
						frameResources[i]->CreateConstantBuffers(device, physicalDevice, scenes);
					}

					Fox::Graphics::Managers::Vulkan::DescriptorManager& descriptorManager =
						Fox::Graphics::Managers::Vulkan::DescriptorManager::Get();

					descriptorManager.Initialize(device, config, static_cast<uint32_t>(scenes.size()));

					for (auto& frameResource : frameResources)
					{

						frameResource->offscreenDescriptorSet = std::make_unique<Fox::Graphics::Vulkan::DescriptorSet>(descriptorManager.GetDescriptorSet(Fox::Graphics::Managers::Vulkan::Descriptor::OFFSCREEN)->AllocateSet());


						for (auto& scene : scenes) {
							auto& constantBuffers = frameResource->GetSceneConstantBuffers(scene->GetID());
							constantBuffers.meshInfosUBO->Update(scene->GetSceneMeshInfos());

							constantBuffers.perFrameDescriptorSet = std::make_unique<Fox::Graphics::Vulkan::DescriptorSet>(descriptorManager.GetDescriptorSet(Fox::Graphics::Managers::Vulkan::Descriptor::MAIN_MESH_BINDLESS_TEXTURING_SHADER)
								->AllocateSet(Fox::Graphics::Managers::Vulkan::MaterialManager::NUM_TEXTURES_PER_BINDLESS_ARRAY));
							constantBuffers.perFrameDescriptorSet->Reserve(7);

							Fox::Graphics::Managers::Vulkan::FrameResourceManager::Get().UpdateConstantBuffersForScene<Fox::Graphics::Geometry::SpriteMaterialInstance, uint32_t>(frameResource->frameIndex, scene.get());
						}

						frameResource->offscreenDescriptorSet->ClearWrites();
						frameResource->offscreenDescriptorSet->SetConstantBuffer(0, frameResource->oldPerFrameUBO);
						frameResource->offscreenDescriptorSet->SetShaderResourceTexture(1, Fox::Graphics::Managers::Vulkan::TextureManager::Get().GetShaderResourceTexture("box").get());
						frameResource->offscreenDescriptorSet->Update();
					}
					return 1;
				}

				template<class T, class E>
				uint32_t FrameResourceManager::UpdateConstantBuffersForScene(uint32_t frameIndex, Fox::Scene::Scene* scene) {
					auto& constantBuffers = frameResources[frameIndex]->GetSceneConstantBuffers(scene->GetID());
					auto& sceneGraph = scene->GetSceneGraph();
					auto renderables = sceneGraph->CollectByLayer(0);

					struct Batch {
						glm::mat4 modelTransform;
						uint32_t modelIndex;
						uint32_t materialIndex;
					};

					std::vector<Batch> batches;
					std::vector<Fox::Graphics::Vulkan::MeshInfo> meshInfos;	

					for (auto& renderable : renderables) {

						auto* entity = renderable->GetEntity<Fox::Scene::Entity>();
						auto* meshComponent = entity->FindComponent<Fox::Scene::MeshComponent>();
						auto* spriteComponent = entity->FindComponent<Fox::Scene::SpriteComponent>(); 

						if (scene->Is3DScene()) {
							if (!entity || (!meshComponent)) {
								continue;
							}

							batches.push_back({
								entity->transform->worldMatrix,
								meshComponent->modelIndex,
								meshComponent->HasMaterial() ? meshComponent->materialInstanceIndex : 0
								});
							meshComponent->SetDepth(0.0f);
						}
						else {
							if (!entity || (!meshComponent || !spriteComponent)) {
								continue;
							}

							batches.push_back({
								entity->transform->worldMatrix,
								meshComponent->modelIndex,
								spriteComponent->HasMaterial() ? spriteComponent->materialInstanceIndex : 0
							});
							meshComponent->SetDepth(spriteComponent ? -(spriteComponent->renderOrder * 0.1f) : 0.0f);

						}

						meshInfos.insert(meshInfos.end(), meshComponent->GetMeshInfos().begin(), meshComponent->GetMeshInfos().end());

					}


					Fox::Graphics::Vulkan::MeshTransforms meshTransforms{};

					meshTransforms.models = std::vector <glm::mat4>(batches.size());
					for (auto& batch : batches) {
						meshTransforms.models[batch.modelIndex] = batch.modelTransform;

						for (auto& mi : meshInfos) {
							if (mi.modelIndex != batch.modelIndex) {
								continue;
							}
							mi.materialIndex = batch.materialIndex;
						}
					}
					constantBuffers.meshInfosUBO->Update(meshInfos);

					std::vector<Fox::Geometry::SpriteMaterial> actualMaterials;
					auto count = Fox::Graphics::Managers::Vulkan::MaterialManager::Get().GetMaterialCount(Fox::Graphics::Managers::Vulkan::MaterialType::SPRITE_MATERIAL);

					for (auto materialIndex = 0; materialIndex < count; materialIndex++) {
						auto materialInstance = Fox::Graphics::Managers::Vulkan::MaterialManager::Get().GetMaterialInstance(Fox::Graphics::Managers::Vulkan::MaterialType::SPRITE_MATERIAL, materialIndex);
						T* materialInstanceCasted = dynamic_cast<T*>(materialInstance.get());

						Fox::Geometry::SpriteMaterial* material = static_cast<Fox::Geometry::SpriteMaterial*>(materialInstanceCasted->Copy());
						actualMaterials.push_back(*dynamic_cast<Fox::Geometry::SpriteMaterial*>(material));
						delete material;
					} 

					constantBuffers.materialsUBO->Update<Fox::Geometry::SpriteMaterial>(actualMaterials);
					constantBuffers.meshTransformsUBO->Update(meshTransforms.models);

					UpdateConstantBuffersReferences(frameIndex, scene, true);

					return meshInfos.size();
				}

				void FrameResourceManager::UpdateConstantBuffersReferences(uint32_t frameIndex, Fox::Scene::Scene* scene, bool updateBindless = false) {
					auto& frameResource = frameResources[frameIndex];
					auto& constantBuffers = frameResource->GetSceneConstantBuffers(scene->GetID());

					constantBuffers.perFrameDescriptorSet->ClearWrites();
					constantBuffers.perFrameDescriptorSet->SetVertexBuffer<Fox::Graphics::Vulkan::Vertex>(0, scene->GetVertexBuffer());
					constantBuffers.perFrameDescriptorSet->SetIndexBuffer<uint32_t>(1, scene->GetIndexBuffer());
					constantBuffers.perFrameDescriptorSet->SetConstantBuffer(2, constantBuffers.perFrameUBO);
					constantBuffers.perFrameDescriptorSet->SetDynamicConstantBuffer(3, constantBuffers.meshTransformsUBO);
					constantBuffers.perFrameDescriptorSet->SetDynamicStorageBuffer(4, constantBuffers.meshInfosUBO);
					constantBuffers.perFrameDescriptorSet->SetDynamicConstantBuffer(5, constantBuffers.materialsUBO);
					
					if (updateBindless) {
						constantBuffers.perFrameDescriptorSet->SetBindlessTextureArray(6, Fox::Graphics::Managers::Vulkan::MaterialManager::Get().GetBindlessTextureArray());
					}
					
					constantBuffers.perFrameDescriptorSet->Update();
				}
			}
		}
	}
}