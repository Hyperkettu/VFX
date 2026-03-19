#pragma once

namespace Fox {

    namespace Graphics {

        namespace Vulkan {

            class Camera
            {
            public:
                Camera()
                {
                    UpdateViewMatrix();
                    UpdateProjectionMatrix();
                }

                virtual ~Camera() = default;

                virtual void MoveForward(float delta)
                {

                }

                virtual void MoveBackward(float delta)
                {
                }

                virtual void Zoom(float amount) = 0;

                virtual void MoveRight(float delta) = 0;
                virtual void MoveLeft(float delta) = 0;
                virtual void MoveUp(float delta) = 0;
                virtual void MoveDown(float delta) = 0;

                virtual void Rotate(float yawDelta, float pitchDelta) = 0;

                virtual void SetPosition(const glm::vec3& pos) = 0;
                virtual const glm::vec3 GetPosition() const = 0;

                virtual const glm::mat4& GetViewMatrix() { return view; }
                virtual const glm::mat4& GetProjectionMatrix() { return projection; }

            protected:

                virtual void UpdateViewMatrix() {};
                virtual void UpdateProjectionMatrix() {};

                glm::mat4 view{1.0f};
                glm::mat4 projection{1.0f};
                bool isDirty;

            };

        }
    }
}