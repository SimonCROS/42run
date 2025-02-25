//
// Created by loumarti on 1/15/25.
//

module;

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

module Engine;
import :Transform;

Camera::Camera(Object& object, const uint32_t width, const uint32_t height, const float fov) : Component(object), m_mode(COLOR)
{
    const float aspect = static_cast<float>(width) / static_cast<float>(height);

    m_projectionMatrix = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
}

auto Camera::computeViewMatrix() const -> glm::mat4
{
    const Transform& transform = object().transform();
    const glm::vec3 forward = transform.rotation() * glm::vec3(0.0f, 0.0f, -1.0f);
    const glm::vec3 up = transform.rotation() * glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 center = transform.translation() + forward;
    return glm::lookAt(transform.translation(), center, up);
}
