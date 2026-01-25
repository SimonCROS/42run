//
// Created by Simon Cros on 1/17/25.
//

module Engine;
import :Transform;
import glm;

auto Transform::setTranslation(const glm::vec3& translation) -> void
{
    m_translation = translation;
    m_object.get().markDirty();
}

auto Transform::translate(const glm::vec3& translation) -> void
{
    m_translation += translation;
    m_object.get().markDirty();
}

auto Transform::setRotation(const glm::quat& rotation) -> void
{
    m_rotation = rotation;
    m_object.get().markDirty();
}

auto Transform::rotate(const glm::quat& rotation) -> void
{
    m_rotation *= rotation;
    m_object.get().markDirty();
}

auto Transform::setScale(const glm::vec3& scale) -> void
{
    m_scale = scale;
    m_object.get().markDirty();
}

auto Transform::scale(const glm::vec3& scale) -> void
{
    m_scale *= scale;
    m_object.get().markDirty();
}

auto Transform::scale(const float scale) -> void
{
    m_scale *= scale;
    m_object.get().markDirty();
}
