//
// Created by Simon Cros on 1/17/25.
//

module;

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

export module Engine:Transform;
import std;

export class Object;

export class Transform
{
private:
    std::reference_wrapper<Object> m_object;
    glm::vec3 m_translation{};
    glm::quat m_rotation = glm::identity<glm::quat>();
    glm::vec3 m_scale{1.0f};

public:
    explicit Transform(Object& object)
        : m_object(object)
    {
    }

    [[nodiscard]] auto translation() const -> glm::vec3 { return m_translation; }
    auto setTranslation(const glm::vec3& translation) -> void;
    auto translate(const glm::vec3& translation) -> void;

    [[nodiscard]] auto rotation() const -> glm::quat { return m_rotation; }
    auto setRotation(const glm::quat& rotation) -> void;
    auto rotate(const glm::quat& rotation) -> void;

    [[nodiscard]] auto scale() const -> glm::vec3 { return m_scale; }
    auto setScale(const glm::vec3& scale) -> void;
    auto scale(const glm::vec3& scale) -> void;
    auto scale(float scale) -> void;

    [[nodiscard]] auto trs() const -> glm::mat4
    {
        auto mat = glm::identity<glm::mat4>();
        mat = glm::translate(mat, m_translation);
        mat *= glm::mat4_cast(m_rotation);
        mat = glm::scale(mat, m_scale);
        return mat;
    }

    friend auto swap(Transform& a, Transform& b) noexcept -> void
    {
        std::swap(a.m_object, b.m_object);
        std::swap(a.m_translation, b.m_translation);
        std::swap(a.m_rotation, b.m_rotation);
        std::swap(a.m_scale, b.m_scale);
    }
};
