//
// Created by Simon Cros on 1/17/25.
//

#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

class Object;

class Transform
{
    friend class Object;

private:
    bool m_dirty{true};
    glm::vec3 m_translation{};
    glm::quat m_rotation = glm::identity<glm::quat>();
    glm::vec3 m_scale{1.0f};

public:
    [[nodiscard]] auto translation() const -> glm::vec3 { return m_translation; }
    auto setTranslation(const glm::vec3& translation) -> void
    {
        m_translation = translation;
        m_dirty = true;
    }

    [[nodiscard]] auto rotation() const -> glm::quat { return m_rotation; }
    auto setRotation(const glm::quat& rotation) -> void
    {
        m_rotation = rotation;
        m_dirty = true;
    }

    [[nodiscard]] auto scale() const -> glm::vec3 { return m_scale; }
    auto setScale(const glm::vec3& scale) -> void
    {
        m_scale = scale;
        m_dirty = true;
    }

    [[nodiscard]] auto trs() const -> glm::mat4
    {
        auto mat = glm::identity<glm::mat4>();
        mat = glm::translate(mat, m_translation);
        mat *= glm::mat4_cast(m_rotation);
        mat = glm::scale(mat, m_scale);
        return mat;
    }
};

#endif //TRANSFORM_H
