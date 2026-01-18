//
// Created by Simon Cros on 29/01/2025.
//

module;

#include <cassert>
#include "tiny_gltf.h"

module Components;
import std;
import Engine;
import Engine.AnimationChannel;

Animator::Animator(Object &object, const Model &mesh): Component(object), m_mesh(mesh)
{
    m_nodeTransforms.resize(mesh.renderInfo().nodesCount);
}

void Animator::onUpdate(Engine &engine)
{
    if (m_animationChanged)
    {
        m_timeSinceAnimationStart = DurationType::zero();
        m_animationChanged = false;
        std::fill(m_nodeTransforms.begin(), m_nodeTransforms.end(), AnimatedTransform{});
    } else
    {
        m_timeSinceAnimationStart += engine.frameInfo().deltaTime;
    }

    if (m_currentAnimationIndex < 0)
        return;

    const Animation &animation = m_mesh.animations()[m_currentAnimationIndex];

    const float animationTime = fmodf(m_timeSinceAnimationStart.count(), animation.duration());

    for (int i = 0; i < animation.channelsCount(); ++i)
    {
        auto &channel = animation.channel(i);
        auto &nodeTransform = m_nodeTransforms[channel.node];

        switch (channel.type)
        {
            case AnimationChannelType::Translation:
                nodeTransform.translation = animation.sampler(channel.sampler).vec3(animationTime);
                break;
            case AnimationChannelType::Rotation:
                nodeTransform.rotation = animation.sampler(channel.sampler).quat(animationTime);
                break;
            case AnimationChannelType::Scale:
                nodeTransform.scale = animation.sampler(channel.sampler).vec3(animationTime);
                break;
            default:
                assert(false);
        }
    }
}
