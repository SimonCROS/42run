//
// Created by Simon Cros on 29/01/2025.
//

export module Components:Animator;
import std.compat;
import glm;
import Engine;
import Engine.Animation;
import Time;

export class Animator final : public Component
{
public:
    struct AnimatedTransform
    {
        std::optional<glm::vec3> translation;
        std::optional<glm::quat> rotation;
        std::optional<glm::vec3> scale;
    };

private:
    bool m_animationChanged{false};
    int m_currentAnimationIndex{-1};
    DurationType m_timeSinceAnimationStart{DurationType::zero()};

    const Model & m_mesh;

    std::vector<AnimatedTransform> m_nodeTransforms;

public:
    explicit
    Animator(Object & object, const Model & mesh);

    auto onUpdate(Engine & engine) -> void override;

    auto setAnimation(const int index) -> void
    {
        if (std::cmp_less(index, -1) || std::cmp_greater_equal(index, m_mesh.animations().size()))
            throw std::out_of_range("Animation index is invalid");
        m_currentAnimationIndex = index;
        m_animationChanged = true;
    }

    [[nodiscard]] auto nodeTransform(const int node) const -> const AnimatedTransform &
    {
        return m_nodeTransforms[node];
    }

    [[nodiscard]] auto mesh() const -> const Model &
    {
        return m_mesh;
    }

    [[nodiscard]] auto animations() const -> const std::vector<Animation> &
    {
        return m_mesh.animations();
    }

    [[nodiscard]] auto currentAnimationIndex() const -> int { return m_currentAnimationIndex; }
};
