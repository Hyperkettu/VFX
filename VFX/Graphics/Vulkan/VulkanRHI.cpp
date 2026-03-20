#include "FoxRenderer.h"
#include "VFX/Scene/MeshComponent.h"

namespace Fox {

	int Fox::Graphics::Vulkan::VulkanRHI::delayMillis = 16;

	namespace Graphics {

		namespace Vulkan {

			VulkanRHI::VulkanRHI() {
			}

			VulkanRHI::~VulkanRHI() {
				vkDeviceWaitIdle(device);
				Destroy();
			}

			int32_t VulkanRHI::Initialize(const Fox::Graphics::RendererConfig& config) {
				this->config = config;

				auto data = Fox::Core::FileSystem::ReadFile("Config/config.txt");
				auto lines = split(data, '\n');
				auto percData = split(lines[0],':')[1];
				Fox::Scene::Main2DScene::imageDimensionPercentage = atof(percData.c_str());

				auto delay = split(lines[1], ':')[1];
				delayMillis = atoi(delay.c_str());

				// Create Vulkan instance
				VkApplicationInfo appInfo{};
				appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				appInfo.pApplicationName = "Fox Vulkan Renderer";
				appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
				appInfo.pEngineName = "No Engine";
				appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
				appInfo.apiVersion = VK_API_VERSION_1_4;

				VkInstanceCreateInfo instanceCreateInfo{};
				instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				instanceCreateInfo.pApplicationInfo = &appInfo;
				instanceCreateInfo.enabledExtensionCount = config.extensionCount;
				instanceCreateInfo.ppEnabledExtensionNames = config.instanceExtensions.data();
				instanceCreateInfo.enabledLayerCount = config.layerCount;
				instanceCreateInfo.ppEnabledLayerNames = config.layers.data();

				VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
				if (result != VK_SUCCESS) {
					std::cerr << "vkCreateInstance failed: " << result << "\n";
					return EXIT_FAILURE;
				}

#ifdef _DEBUG
				CreateDebugCallback();
#endif

				if (!SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(config.windowHandle), instance, &surface)) {
					std::cerr << "SDL_Vulkan_CreateSurface failed\n";
					return 0;
				}

				// currently just choose the first one
				if (!PickPhysicalDevice()) {
					std::cerr << "Picking physical device failed\n";
					return 0;
				}

				if (!FindGraphicsQueueFamily()) {
					std::cerr << "Finding graphics queue family failed\n";
					return 0;
				}

				if (!CreateLogicalDevice()) {
					std::cerr << "Creating logical device failed\n";
					return 0;
				}

				LoadExtensionAPIFunctions();

				if (!GetGraphicsQueue()) {
					std::cerr << "Getting graphics queue failed\n";
					return 0;
				}
				if (!GetPresentQueue()) {
					std::cerr << "Getting present queue failed\n";
					return 0;
				}

				if (!GetSurfaceFormat()) {
					std::cerr << "Getting surface format failed\n";
					return 0;
				}

				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
				auto commandPool = std::make_unique<Fox::Graphics::Vulkan::CommandPool>(device, graphicsQueueFamily);

				Fox::Graphics::Managers::Vulkan::TextureManager::Get().Initialize(device, physicalDevice, capabilities, surfaceFormat, depthFormat, graphicsQueue, graphicsQueueFamily, commandPool);

				Fox::Graphics::Managers::Vulkan::RenderPassManager::Get().Initialize(
					device,
					capabilities,
					surfaceFormat,
					depthFormat
				);

				Fox::Graphics::Managers::Vulkan::MaterialManager::Get().Initialize(device);
				Fox::Graphics::Managers::Vulkan::FrameResourceManager::Get().Initialize(device, config, capabilities, surface, surfaceFormat, depthFormat, graphicsQueueFamily);
				Fox::Graphics::Managers::Vulkan::MeshManager::Get().Initialize(device);
				Fox::Graphics::Managers::Vulkan::SceneManager::Get().Initialize(device, physicalDevice, commandPool, graphicsQueue, capabilities);

				if (!Fox::Graphics::Managers::Vulkan::FrameResourceManager::Get().CreateUniformBuffers(config, physicalDevice)) {
					std::cerr << "Creating uniform buffer failed\n";
					return 0;
				}

				Fox::Graphics::Managers::Vulkan::PipelineManager::Get().Initialize(device, capabilities);
				Fox::Graphics::Managers::Vulkan::ParticleEffectManager::Get().Initialize(device, physicalDevice, config.MAX_FRAMES_IN_FLIGHT);

				std::cout << "Vulkan instance created successfully.\n";
				return 1;
			}

			int32_t VulkanRHI::Destroy() {
				std::cout << "Destroy Vulkan RHI" << std::endl;

				Fox::Graphics::Managers::Vulkan::FrameResourceManager::Get().Destroy();
				Fox::Graphics::Managers::Vulkan::RenderPassManager::Get().Destroy(); 
				Fox::Graphics::Managers::Vulkan::PipelineManager::Get().Destroy();
				Fox::Graphics::Managers::Vulkan::DescriptorManager::Get().Destroy();
				Fox::Graphics::Managers::Vulkan::TextureManager::Get().Destroy();
				Fox::Graphics::Managers::Vulkan::MeshManager::Get().Destroy();
				Fox::Graphics::Managers::Vulkan::SceneManager::Get().Destroy();
				Fox::Graphics::Managers::Vulkan::ParticleEffectManager::Get().Destroy();

				vkDestroyDevice(device, nullptr);
				vkDestroySurfaceKHR(instance, surface, nullptr);

#ifdef _DEBUG
				DestroyDebugCallback();
#endif

				vkDestroyInstance(instance, nullptr);
				return 1;
			}

			int32_t VulkanRHI::PickPhysicalDevice() {
				// Enumerate physical devices
				uint32_t gpuCount = 0;
				vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
				if (gpuCount == 0) {
					std::cerr << "No Vulkan-compatible GPU found.\n";
					return 0;
				}
				std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
				vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
				physicalDevice = physicalDevices[0];

				return 1;
			}

			int32_t VulkanRHI::FindGraphicsQueueFamily() {
				// Find graphics queue family
				uint32_t queueFamilyCount = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
				std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

				graphicsQueueFamily = -1;
				for (int i = 0; i < queueFamilies.size(); ++i) {
					VkBool32 presentSupport = false;
					vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
					if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport && queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
						graphicsQueueFamily = i;
						break;
					}
				}
				if (graphicsQueueFamily == -1) {
					std::cerr << "Failed to find suitable queue family.\n";
					return 0;
				}

				return 1;
			}

			int32_t VulkanRHI::CreateLogicalDevice() {
				float queuePriority = 1.0f;
				VkDeviceQueueCreateInfo queueCreateInfo{};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = graphicsQueueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;

				const char* deviceExtensions[] = {
					VK_KHR_SWAPCHAIN_EXTENSION_NAME,
					VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
					VK_EXT_MESH_SHADER_EXTENSION_NAME,
					VK_KHR_SPIRV_1_4_EXTENSION_NAME,
					VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
					VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME // for bindless rendering
				};

				// --- Maintenance4 feature (if desired) ---
				VkPhysicalDeviceMaintenance4Features maintenance4 = {};
				maintenance4.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
				maintenance4.pNext = nullptr;
				maintenance4.maintenance4 = VK_TRUE; // if you need it


				// --- Vulkan 1.2 feature struct (core 1.2 features) ---
				VkPhysicalDeviceVulkan12Features vk12 = {};
				vk12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
				vk12.descriptorIndexing = VK_TRUE;
				vk12.runtimeDescriptorArray = VK_TRUE;
				vk12.descriptorBindingVariableDescriptorCount = VK_TRUE;
				vk12.descriptorBindingPartiallyBound = VK_TRUE;
				vk12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
				vk12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
				vk12.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
				vk12.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
				vk12.shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE;
				vk12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE; // if you plan update-after-bind for sampled images
				vk12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE; // if needed
				vk12.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE; // usually false (UBO updates-after-bind are rarely needed)
				vk12.descriptorBindingStorageBufferUpdateAfterBind = VK_FALSE; // only if you need it

				vk12.pNext = &maintenance4; // chain to descriptor indexing features

				// Request the 1.2 fields required by your SPIR-V:
				vk12.runtimeDescriptorArray = VK_TRUE;                    // maps to SPIR-V RuntimeDescriptorArray
				vk12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE; // maps to sampled image non-uniform indexing

				// --- Mesh shader features (EXT) ---
				VkPhysicalDeviceMeshShaderFeaturesEXT meshShader = {};
				meshShader.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
				meshShader.pNext = &vk12;
				meshShader.meshShader = VK_TRUE;
				meshShader.taskShader = VK_TRUE; // if you also want task shaders

				// --- Device create info ---
				VkDeviceCreateInfo deviceCreateInfo = {};
				deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				deviceCreateInfo.pNext = &meshShader; // chain head

				deviceCreateInfo.queueCreateInfoCount = 1;
				deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

				deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(6);
				deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;


				if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
					std::cerr << "Failed to create logical device.\n";
					return 0;
				}

				return 1;
			}

			int32_t VulkanRHI::GetGraphicsQueue() {
				vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
				return 1;
			}

			int32_t VulkanRHI::GetPresentQueue() {
				vkGetDeviceQueue(device, graphicsQueueFamily, 0, &presentQueue);
				return 1;
			}

			int32_t VulkanRHI::GetSurfaceFormat() {
				uint32_t formatCount;
				vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
				std::vector<VkSurfaceFormatKHR> formats(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

				// just pick first
				surfaceFormat = formats[0];

				return 1;
			}

			void VulkanRHI::Render() {
				auto& frameResourceManager = Fox::Graphics::Managers::Vulkan::FrameResourceManager::Get();
				auto& scenes = Graphics::Managers::Vulkan::SceneManager::Get().GetActiveScenes();

				auto& camera = scenes[0]->GetMainCamera();
				auto& frameResource = frameResourceManager.GetFrameResource(currentFrame);

				if (inputManager->IsKeyDown(SDL_SCANCODE_A)) {
					camera->MoveLeft(cameraSpeed * deltaTime);
				}
				if (inputManager->IsKeyDown(SDL_SCANCODE_D)) {
					camera->MoveRight(cameraSpeed * deltaTime);
				}
				if (inputManager->IsKeyDown(SDL_SCANCODE_W)) {
					camera->MoveForward(cameraSpeed * deltaTime);
				}
				if (inputManager->IsKeyDown(SDL_SCANCODE_S)) {
					camera->MoveBackward(cameraSpeed * deltaTime);
				}
				if (inputManager->IsKeyDown(SDL_SCANCODE_R)) {
					camera->MoveUp(cameraSpeed * deltaTime);
				}
				if (inputManager->IsKeyDown(SDL_SCANCODE_F)) {
					camera->MoveDown(cameraSpeed * deltaTime);
				}

				for (auto& scene : scenes) {
					scene->Update(deltaTime);
				}

				// TODO add depth pre-pass to compute collisions

				Fox::Graphics::Managers::Vulkan::ParticleEffectManager::Get().Update(frameResource->computeCommandPool, frameResource->computeCommandList, graphicsQueue, currentFrame, deltaTime);

				frameResource->renderFence->Wait();
				frameResource->renderFence->Reset();

				Graphics::Managers::Vulkan::TextureManager::Get().Update(currentFrameIndex);

				std::array<VkClearValue, 2> clearValues{};
				clearValues[0].color = { 0.0f, 0.0f, 0.34f, 1.0f };
				clearValues[1].depthStencil = { 1.0f, 0 };

				frameResource->commandPool->Reset();
				
				uint32_t imageIndex = frameResourceManager.GetSwapchain()->AcquireNextImage(frameResource->imageAvailableSemaphore->Get());

				Fox::Graphics::Managers::Vulkan::ParticleEffectManager::Get().Render(frameResource->commandPool, frameResource->commandList, graphicsQueue, frameResource->imageAvailableSemaphore, currentFrame, imageIndex, capabilities, clearValues);

				vkDeviceWaitIdle(device);

				frameResource->commandPool->Reset();

				auto& commandList = frameResource->commandList->Begin();

				for (size_t sceneIndex = 0; sceneIndex < scenes.size(); sceneIndex++)
				{
					auto& scene = scenes[sceneIndex];
					auto& constantBuffers = frameResource->GetSceneConstantBuffers(scene->GetID());

					frameResourceManager.UpdateConstantBuffersReferences(currentFrame, scene.get(), false);
					uint32_t numRenderables = UpdateUniformBuffer(currentFrame, scene);

					auto pipeline = Fox::Graphics::Managers::Vulkan::PipelineManager::Get().GetPipeline(
						Fox::Graphics::Managers::Vulkan::PipelineCategory::MESH_SHADER_BINDLESS_TEXTURING_NO_CLEAR)->Get();

					auto layout = Fox::Graphics::Managers::Vulkan::PipelineManager::Get().GetPipeline(Fox::Graphics::Managers::Vulkan::PipelineCategory::MESH_SHADER_BINDLESS_TEXTURING_NO_CLEAR)->GetLayout();

					commandList
						.BeginRenderPass(
							Fox::Graphics::Managers::Vulkan::RenderPassManager::Get().GetPass(
								Fox::Graphics::Managers::Vulkan::RenderPass::TO_PRESENTABLE_NO_CLEAR)->Get(), // DEFAULT NO CLEAR
							frameResourceManager.GetSwapchain()->GetFramebuffer(imageIndex),
							capabilities.currentExtent,
							nullptr, 0)
						.SetViewport(0, 0, capabilities.currentExtent.width, capabilities.currentExtent.height)
						.SetScissor(0, 0, capabilities.currentExtent.width, capabilities.currentExtent.height)
						.BindPipeline(pipeline)
						.BindDescriptorSets(layout, 0, { constantBuffers.perFrameDescriptorSet->Get() })
						.BindVertexBuffers(0, { scene->GetVertexBuffer()->Get() }, { 0 })
						.BindIndexBuffer(scene->GetIndexBuffer()->Get(), 0, VK_INDEX_TYPE_UINT32)
						.RenderMeshShader(numRenderables, 1, 1)
						.EndRenderPass();
				}

				commandList.End();

				commandList.Submit(
					graphicsQueue,
					Fox::Graphics::Managers::Vulkan::ParticleEffectManager::Get().GetParticlesFinishedSemaphore(currentFrame),
					frameResource->renderFinishedSemaphore->Get(),
					frameResource->renderFence->Get());

				Present(frameResourceManager.GetSwapchain(), imageIndex, frameResource->renderFinishedSemaphore);

				currentFrame = (currentFrame + 1) % config.MAX_FRAMES_IN_FLIGHT;
			}

			const char* GetDebugSeverity(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
				switch (severity)
				{
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
					return "Verbose";

				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
					return "Info";

				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
					return "Warning";

				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
					return "Error";

				default:
					std::cout << "Invalid severity code" << severity << std::endl;
					exit(1);
				}

				return "No such severity";
			}

			void VulkanRHI::Present(const std::unique_ptr<Fox::Graphics::Vulkan::Swapchain>& swapchain,
				uint32_t imageIndex, 
				std::unique_ptr<Fox::Graphics::Vulkan::Semaphore>& semaphore) {
				VkSemaphore signalSemaphores[] = { semaphore->Get()};

				VkSwapchainKHR swapchainKHR = swapchain->Get();
				VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
				presentInfo.waitSemaphoreCount = 1;
				presentInfo.pWaitSemaphores = signalSemaphores;
				presentInfo.swapchainCount = 1;
				presentInfo.pSwapchains = &swapchainKHR;
				presentInfo.pImageIndices = &imageIndex;

				vkQueuePresentKHR(presentQueue, &presentInfo);
			}

			const char* GetDebugType(VkDebugUtilsMessageTypeFlagsEXT type) {
				switch (type)
				{
				case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
					return "General";

				case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
					return "Validation";

				case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
					return "Performance";

				case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
					return "Device address binding";

				default:
					std::cout << "Invalid type code" << type << std::endl;;
					exit(1);
				}

				return "No such type";
			}

			static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT severity,
				VkDebugUtilsMessageTypeFlagsEXT type,
				const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData) {
				std::cout << "Debug callback: " << pCallbackData->pMessage << std::endl;
				std::cout << "Severity: " << GetDebugSeverity(severity) << std::endl;
				std::cout << "  Type " << GetDebugType(type) << std::endl;
				std::cout << "  Objects " << std::endl;

				for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
					printf("%llx ", pCallbackData->pObjects[i].objectHandle);
				}

				return VK_FALSE;
			}

			void VulkanRHI::CreateDebugCallback() {
				VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
					.pNext = NULL,
					.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
									   VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
									   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
									   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
								   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
								   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
					.pfnUserCallback = &DebugCallback,
					.pUserData = NULL
				};

				PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = VK_NULL_HANDLE;
				vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

				if (!vkCreateDebugUtilsMessenger) {
					std::cout << "Cannot find address of vkCreateDebugUtilsMessenger" << std::endl;
					exit(1);
				}

				VkResult result = vkCreateDebugUtilsMessenger(instance, &messengerCreateInfo, NULL, &debugMessenger);
				if (result != VK_SUCCESS) {
					std::cout << "Creating debug utils messenger failed." << std::endl;
				}

				std::cout << "Debug utils messenger created." << std::endl;
			}

			void VulkanRHI::DestroyDebugCallback() {
				PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
				vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

				if (!vkDestroyDebugUtilsMessenger) {
					std::cout << "Cannot find address of vkDestroyDebugUtilsMessenger" << std::endl;
					exit(1);
				}

				vkDestroyDebugUtilsMessenger(instance, debugMessenger, NULL);

				std::cout << "Debug utils messenger destroyed." << std::endl;
			}

			void VulkanRHI::LoadExtensionAPIFunctions() {
				Fox::Graphics::Vulkan::CommandList::RegisterRenderMeshShaderFunction(reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksEXT")));
			}

			void VulkanRHI::UpdateOffscreenUniformBuffer(uint32_t currentFrame, std::shared_ptr<Fox::Scene::Scene>& scene) {
				auto& frameResource = Fox::Graphics::Managers::Vulkan::FrameResourceManager::Get().GetFrameResource(currentFrame);
				auto& camera = scene->GetMainCamera();

				Fox::Graphics::Vulkan::OldFrame oldPerFrame{};
				oldPerFrame.model = glm::rotate(glm::mat4(1.0f),
					(float)rotationAngle * glm::radians(90.0f),
					glm::vec3(0.0f, 0.0f, 1.0f));
				oldPerFrame.view = camera->GetViewMatrix();
				oldPerFrame.proj = camera->GetProjectionMatrix();

				frameResource->oldPerFrameUBO->Update(oldPerFrame);
			}


			uint32_t VulkanRHI::UpdateUniformBuffer(uint32_t currentFrame, std::shared_ptr<Fox::Scene::Scene>& scene) {
				auto& frameResource = Fox::Graphics::Managers::Vulkan::FrameResourceManager::Get().GetFrameResource(currentFrame);
				auto& constantBuffers = frameResource->GetSceneConstantBuffers(scene->GetID());
				auto& camera = scene->GetMainCamera();

				Fox::Graphics::Vulkan::PerFrame perFrame{};
				perFrame.view = camera->GetViewMatrix();
				perFrame.proj = camera->GetProjectionMatrix();

				constantBuffers.perFrameUBO->Update(perFrame);

				uint32_t numRenderables = Fox::Graphics::Managers::Vulkan::FrameResourceManager::Get().UpdateConstantBuffersForScene<Fox::Graphics::Geometry::SpriteMaterialInstance, uint32_t>(frameResource->frameIndex, scene.get());

				return numRenderables;
			}

			void VulkanRHI::RegisterInput(Fox::Input::InputManager& input) {
				inputManager = &input;

				quitKeyBind = input.OnKeyPressed.Connect([=](SDL_Scancode code) {
					if (code == SDL_SCANCODE_ESCAPE) {
						isQuitting = true;
					}
				});

				input.ToggleRelativeMode();

				float sensitivity = 0.13f;

				mouseInputBind = input.OnMouseMoved.Connect([=](int dx, int dy) {

					if (input.IsRelativeMode()) {
						Fox::Graphics::Managers::Vulkan::SceneManager::Get().GetActiveScenes()[0]->GetMainCamera()->Rotate(static_cast<float>(dx) * sensitivity, -static_cast<float>(dy) * sensitivity);
					}
				});

			}

		}
	}
}