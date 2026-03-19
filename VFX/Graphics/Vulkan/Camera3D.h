#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VFX/Graphics/Vulkan/Camera.h"

namespace Fox {

    namespace Graphics {

        namespace Vulkan {

            class Camera3D : public Fox::Graphics::Vulkan::Camera
            {
            public:

                Camera3D() = default;

                Camera3D(float fovDegrees, float aspectRatio, float nearPlane, float farPlane)
                    : fov(glm::radians(fovDegrees)),
                    aspect(aspectRatio),
                    near(nearPlane),
                    far(farPlane)
                {
                    UpdateViewMatrix();
                    UpdateProjectionMatrix();
                }

                virtual ~Camera3D() = default;

                void MoveForward(float delta)
                {
                    position += front * delta;
                    UpdateViewMatrix();
                }

                void MoveBackward(float delta)
                {
                    position -= front * delta;
                    UpdateViewMatrix();
                }

                void MoveRight(float delta)
                {
                    position += right * delta;
                    UpdateViewMatrix();
                }

                void MoveLeft(float delta)
                {
                    position -= right * delta;
                    UpdateViewMatrix();
                }

                void MoveUp(float delta)
                {
                    position += worldUp * delta;
                    UpdateViewMatrix();
                }

                void MoveDown(float delta)
                {
                    position -= worldUp * delta;
                    UpdateViewMatrix();
                }

                void Rotate(float yawDelta, float pitchDelta)
                {
                    yaw += yawDelta;
                    pitch += pitchDelta;

                    if (pitch > 89.0f) pitch = 89.0f;
                    if (pitch < -89.0f) pitch = -89.0f;

                    UpdateVectors();
                    UpdateViewMatrix();
                }

                void SetPosition(const glm::vec3& pos)
                {
                    position = pos;
                    UpdateViewMatrix();
                }


                virtual void Zoom(float amount) override {
                 
                }

                const glm::vec3 GetPosition() const { return position; }

                void SetAspectRatio(float aspect)
                {
                    aspect = aspect;
                    UpdateProjectionMatrix();
                }

                void SetWorldUp(const glm::vec3& upVector)
                {
                    worldUp = upVector;
                    UpdateVectors();
                    UpdateViewMatrix();
				}

                void SetCameraTarget(const glm::vec3& target)
                {
                    front = glm::normalize(target - position);
                    yaw = glm::degrees(atan2(front.z, front.x));
                    pitch = glm::degrees(asin(front.y));
                    UpdateVectors();
                    UpdateViewMatrix();
				}

            private:
                void UpdateVectors()
                {
                    glm::vec3 front;
                    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); 
                    front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
                    front.y = sin(glm::radians(pitch));                          
                    front = glm::normalize(front);
                    this->front = front;
                    // Recalculate right and up vectors
                    right = glm::normalize(glm::cross(front, worldUp));          
                    up = glm::normalize(glm::cross(right, front));
                }

                virtual void UpdateViewMatrix() override
                {
                    view = glm::lookAt(position, position + front, up);
                }

                virtual void UpdateProjectionMatrix() override
                {
                    projection = glm::perspective(fov, aspect, near, far);
                    // Vulkan: flip Y and adjust depth range
                    projection[1][1] *= -1.0f;
                }

            private:
                glm::vec3 position{0.0f, 0.0f, 5.0f};
                glm::vec3 front{0.0f, 0.0f, -1.0f};
                glm::vec3 up{0.0f, 1.0f, 0.0f};
                glm::vec3 right{1.0f, 0.0f, 0.0f};
                glm::vec3 worldUp{0.0f, 1.0f, 0.0f};

                float yaw = -90.0f;   // facing -Z
                float pitch = 0.0f;

                float fov;
                float aspect;
                float near;
                float far;
            };

        }
    }
}