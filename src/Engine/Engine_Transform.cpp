//
// Created by Simon Cros on 1/17/25.
//

module;

#include "glm/glm.hpp"

module Engine;
import :Transform;

auto Transform::setTranslation(const glm::vec3& translation) -> void
{
    m_translation = translation;
    m_object.markDirty();
}

auto Transform::translate(const glm::vec3& translation) -> void
{
    m_translation += translation;
    m_object.markDirty();
}

auto Transform::setRotation(const glm::quat& rotation) -> void
{
    m_rotation = rotation;
    m_object.markDirty();
}

auto Transform::rotate(const glm::quat& rotation) -> void
{
    m_rotation *= rotation;
    m_object.markDirty();
}

auto Transform::setScale(const glm::vec3& scale) -> void
{
    m_scale = scale;
    m_object.markDirty();
}

auto Transform::scale(const glm::vec3& scale) -> void
{
    m_scale *= scale;
    m_object.markDirty();
}
