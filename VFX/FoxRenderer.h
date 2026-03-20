#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <array>
#include <thread>
#include <mutex>
#include <condition_variable>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include<glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "STB_image/stb_image.h"

#include "VFX/Core/Singleton.h"
#include "VFX/Core/Connection.h"
#include "VFX/Core/Signal.h"

#include "VFX/Graphics/Vulkan/Vertex.h"
#include "VFX/Input/InputManager.h"
#include "VFX/Graphics/Vulkan/Camera.h"
#include "VFX/Graphics/Vulkan/Camera3D.h"
#include "VFX/Graphics/Vulkan/Camera2D.h"
#include "VFX/Core/FileSystem.h"
#include "VFX/Graphics/RendererConfig.h"
#include "VFX/Graphics/RHI.h"
#include "VFX/Graphics/Vulkan/CommandPool.h"
#include "VFX/Graphics/Vulkan/CommandList.h"
#include "VFX/Graphics/Vulkan/ConstantBuffers.h"
#include "VFX/Graphics/Vulkan/ShaderStorageBuffer.h"
#include "VFX/Graphics/Vulkan/Buffer.h"
#include "VFX/Graphics/Vulkan/ConstantBuffer.h"
#include "VFX/Graphics/Vulkan/DynamicBuffer.h"
#include "VFX/Graphics/Vulkan/DynamicConstantBuffer.h"
#include "VFX/Graphics/Vulkan/VertexBuffer.h"
#include "VFX/Graphics/Vulkan/IndexBuffer.h"
#include "VFX/Graphics/Vulkan/Fence.h"
#include "VFX/Graphics/Vulkan/Semaphore.h"
#include "VFX/Graphics/Vulkan/Texture.h"
#include "VFX/Graphics/Vulkan/ShaderResourceTexture.h"
#include "VFX/Graphics/Vulkan/RenderTargetTexture.h"
#include "VFX/Graphics/Vulkan/DepthTexture.h"
#include "VFX/Graphics/Vulkan/SwapchainTexture.h"
#include "VFX/Graphics/Managers/Vulkan/TextureManager.h"
#include "VFX/Graphics/Vulkan/Framebuffer.h"
#include "VFX/Graphics/Vulkan/Swapchain.h"
#include "VFX/Graphics/Vulkan/DescriptorPool.h"
#include "VFX/Graphics/Vulkan/DescriptorSetLayout.h"
#include "VFX/Graphics/Vulkan/DescriptorSet.h"
#include "VFX/Graphics/Vulkan/DescriptorSetBuilder.h"
#include "VFX/Graphics/Vulkan/RenderPass.h"
#include "VFX/Graphics/Vulkan/RenderPassBuilder.h"
#include "VFX/Graphics/Vulkan/PipelineLayout.h"
#include "VFX/Graphics/Vulkan/Pipeline.h"
#include "VFX/Graphics/Vulkan/ComputePipeline.h"
#include "VFX/Graphics/Vulkan/PipelineBuilder.h"

#include "VFX/Graphics/Geometry/Submesh.h"
#include "VFX/Graphics/Geometry/Vulkan/Mesh.h"
#include "VFX/Graphics/Geometry/GeometryGenerator.h"
#include "VFX/Graphics/Vulkan/FrameResource.h"


#include "VFX/Graphics/Managers/Vulkan/DescriptorManager.h"
#include "VFX/Graphics/Managers/Vulkan/RenderPassManager.h"
#include "VFX/Graphics/Managers/Vulkan/PipelineManager.h"
#include "VFX/Graphics/Managers/Vulkan/MeshManager.h"
#include "VFX/Graphics/Managers/Vulkan/FrameResourceManager.h"
#include "VFX/Graphics/Managers/Vulkan/MaterialManager.h"
#include "VFX/Graphics/Vulkan/ParticleEffect.h"
#include "VFX/Graphics/Vulkan/EulerParticleEffect.h"
#include "VFX/Graphics/Managers/Vulkan/ParticleEffectManager.h"

#include "VFX/Scene/Hierarchy.h"
#include "VFX/Scene/Scene.h"
#include "VFX/Scene/Scene2D.h"
#include "VFX/Scene/Scene3D.h"
#include "VFX/Scene/MainScene.h"
#include "VFX/Scene/Main2DScene.h"

#include "VFX/Graphics/Managers/Vulkan/SceneManager.h"


#include "VFX/Graphics/Vulkan/VulkanRHI.h"
#include "VFX/Platform/IApplication.h"
#include "VFX/Platform/Vulkan/Application.h"
