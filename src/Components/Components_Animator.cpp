//
// Created by Simon Cros on 29/01/2025.
//

module;

#include <algorithm>

#include "tiny_gltf.h"

module Components;
import Engine;

Animator::Animator(Object& object, const Mesh& mesh): Component(object), m_mesh(mesh)
{
    m_nodeTransforms.resize(mesh.model().nodes.size());
}

void Animator::onUpdate(Engine& engine)
{
    if (m_animationChanged)
    {
        m_timeSinceAnimationStart = DurationType::zero();
        m_animationChanged = false;
        std::fill(m_nodeTransforms.begin(), m_nodeTransforms.end(), AnimatedTransform{});
    }
    else
    {
        m_timeSinceAnimationStart += engine.frameInfo().deltaTime;
    }

    if (m_currentAnimationIndex < 0)
        return;

    const tinygltf::Animation& gltfAnimation = m_mesh.model().animations[m_currentAnimationIndex];
    const Animation& animation = m_mesh.animations()[m_currentAnimationIndex];

    const float animationTime = fmodf(m_timeSinceAnimationStart.count(), animation.duration());

    for (const auto& channel : gltfAnimation.channels)
    {
        auto& nodeTransform = m_nodeTransforms[channel.target_node];

        if (channel.target_path == "rotation")
            nodeTransform.rotation = animation.sampler(channel.sampler).quat(animationTime);
        else if (channel.target_path == "scale")
            nodeTransform.scale = animation.sampler(channel.sampler).vec3(animationTime);
        else if (channel.target_path == "translation")
            nodeTransform.translation = animation.sampler(channel.sampler).vec3(animationTime);
    }
}
