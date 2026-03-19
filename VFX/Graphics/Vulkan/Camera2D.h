#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VFX/Graphics/Vulkan/Camera.h"

namespace Fox {

    namespace Graphics {

        namespace Vulkan {

            class Camera2D : public Fox::Graphics::Vulkan::Camera
            {
            public:

                Camera2D() = default;

                Camera2D(float viewportWidth = 1280.0f, float viewportHeight = 720.0f)
                    : position(0.0f, 0.0f),
                    rotation(0.0f),
                    zoom(1.0f),
                    viewportWidth(viewportWidth),
                    viewportHeight(viewportHeight)
                {
                    RecalculateMatrices();
                }

                virtual ~Camera2D() = default;

                virtual void MoveForward(float delta) override
                {
                }

                virtual void MoveBackward(float delta) override
                {
                }

                virtual void MoveRight(float delta) override
                {
                    position += right * delta;
                    isDirty = true;
                }

                virtual void MoveLeft(float delta) override
                {
                    position -= right * delta;
                    isDirty = true;
                }

                virtual void MoveUp(float delta) override
                {
                    position += up * delta;
                    isDirty = true; 
                }

                virtual void MoveDown(float delta) override
                {
                    position -= up * delta;
                    isDirty = true;
                }

                virtual void Rotate(float yawDelta, float pitchDelta) override
                {
                    rotation += yawDelta;
                    SetRotation(rotation);
                }

                void SetPosition(const glm::vec3& pos)
                {
                    position = pos;
                    isDirty = true;
                }

                //----------------------------------------------------------------------
                // Setters
                //----------------------------------------------------------------------
                void SetPosition(const glm::vec2& pos) {
                    position = pos;
                    isDirty = true;
                }

                void SetRotation(float degrees) {
                    rotation = degrees;
                    isDirty = true;
                }

                virtual const glm::vec3 GetPosition() const override { return glm::vec3(position, 0.0f); }


                virtual void Zoom(float amount) override {
                    zoom = glm::max(0.001f, amount);
                    isDirty = true;
                }

                void SetViewport(float width, float height) {
                    viewportWidth = width;
                    viewportHeight = height;
                    isDirty = true;
                }

                //----------------------------------------------------------------------
                // Get combined view-projection matrix
                //----------------------------------------------------------------------
                const glm::mat4& GetViewProjectionMatrix() {
                    if (isDirty) RecalculateMatrices();
                    return viewProjection;
                }

                virtual const glm::mat4& GetViewMatrix() override {
                    if (isDirty) RecalculateMatrices();
                    return view;
                }

                virtual const glm::mat4& GetProjectionMatrix() override{
                    if (isDirty) RecalculateMatrices();
                    return projection;
                }

                //----------------------------------------------------------------------
                // Helper: reset camera for overlay use
                //----------------------------------------------------------------------
                void MakeOverlayCamera() {
                    position = glm::vec2(0.0f);
                    rotation = 0.0f;
                    zoom = 1.0f;
                    isDirty = true; // recalc needed
                }

            protected:

                virtual void UpdateViewMatrix() override {
                    view = glm::mat4(1.0f);
                }

                virtual void UpdateProjectionMatrix() override {
                
                }

                void RecalculateMatrices()
                {
                    glm::mat4 transform(1.0f);

                     
                    transform = glm::translate(transform, glm::vec3(position, 0.0f));

                    transform = glm::rotate(transform,
                        glm::radians(rotation),
                        glm::vec3(0, 0, 1));

                    transform = glm::scale(transform, glm::vec3(zoom, zoom, 1.0f));

                    view = glm::inverse(transform);

                    projection = glm::ortho(
                        0.0f, viewportWidth,
                        viewportHeight, 0.0f,
                        -1.0f, +1.0f
                    );

                    viewProjection = projection * view;
                    isDirty = false;
                }

                glm::vec2 right = glm::vec2(1.0f, 0.0f);
                glm::vec2 up = glm::vec2(0.0f, 1.0f);

                glm::vec2 position;
                float rotation;
                float zoom;

                float viewportWidth;
                float viewportHeight;

                glm::mat4 viewProjection;

            };

        } 
    }
}