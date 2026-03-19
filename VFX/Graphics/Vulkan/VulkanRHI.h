#pragma once 

namespace Fox {

	namespace Graphics {

		namespace Vulkan {

			class VulkanRHI : public Fox::Graphics::RHI {
			public:
				VulkanRHI();
				virtual ~VulkanRHI();
				int32_t Initialize(const Fox::Graphics::RendererConfig& config) override;
				int32_t Destroy() override;
				virtual void Render() override;

				void RegisterInput(Fox::Input::InputManager& input) override;

				bool IsQuitting() const override {
					return isQuitting;
				}

			protected: 
				int32_t PickPhysicalDevice();
				int32_t FindGraphicsQueueFamily();
				int32_t CreateLogicalDevice();
				int32_t GetGraphicsQueue();
				int32_t GetPresentQueue();
				int32_t GetSurfaceFormat();

				void CreateDebugCallback();
				void DestroyDebugCallback();

				void LoadExtensionAPIFunctions();
				uint32_t UpdateUniformBuffer(uint32_t currentFrame, std::shared_ptr<Fox::Scene::Scene>& scene);
				void UpdateOffscreenUniformBuffer(uint32_t currentFrame, std::shared_ptr<Fox::Scene::Scene>& scene);


				void Present(const std::unique_ptr<Fox::Graphics::Vulkan::Swapchain>& swapchain, uint32_t imageIndex, std::unique_ptr<Fox::Graphics::Vulkan::Semaphore>& semaphore);

			private:
				VkInstance instance;
				VkSurfaceKHR surface;
				VkPhysicalDevice physicalDevice;			
				VkDevice device;
				int32_t graphicsQueueFamily = -1;
				VkQueue graphicsQueue;
				VkQueue presentQueue;
				int32_t currentFrame = 0;
				VkSurfaceFormatKHR surfaceFormat;
				VkSurfaceCapabilitiesKHR capabilities;

				VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

				float rotationAngle = 0.0;

				VkFormat depthFormat = VkFormat::VK_FORMAT_D24_UNORM_S8_UINT;

				Fox::Graphics::RendererConfig config;

				// input
				float cameraSpeed = 5.0f;
				float deltaTime = 1.0f / 60.0f; // TODO add real delta time
				Fox::Input::InputManager* inputManager;
				Fox::Core::Connection mouseInputBind;
				Fox::Core::Connection keyInputBind;

				uint32_t currentFrameIndex = 0;

				Fox::Core::Connection quitKeyBind;
				bool isQuitting = false;

				std::vector<std::string> split(const std::string& s, char delimiter) {
					std::vector<std::string> tokens;
					std::string token;
					std::stringstream ss(s);

					while (std::getline(ss, token, delimiter)) {
						tokens.push_back(token);
					}
					return tokens;
				}

			public:
				static int delayMillis;
			};

		}
	}
}