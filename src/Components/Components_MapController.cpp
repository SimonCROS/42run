//
// Created by Simon Cros on 2/20/25.
//

module;

#include <chrono>
#include <queue>
#include <deque>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

module Components;
import :MeshRenderer;
import Engine;

static auto instantiatePlaneTwoTables(Engine& engine) -> Object&
{
    auto& shaderProgram = engine.getShaderProgram("default")->get();
    auto& floorMesh = engine.getModel("floor")->get();
    auto& deskMesh = engine.getModel("desk")->get();

    // Floor
    auto& object = engine.instantiate();
    object.transform().setTranslation({0, 0, 5});
    object.addComponent<MeshRenderer>(floorMesh, shaderProgram);

    {
        // Desk
        auto& subObject = engine.instantiate();
        subObject.transform().scale(0.004f);
        subObject.transform().setTranslation({-2, 0, 5});
        subObject.transform().setRotation(glm::quat({0, glm::radians(90.0f), 0}));
        subObject.addComponent<MeshRenderer>(deskMesh, shaderProgram);
        subObject.setParent(object);
    }

    {
        // Desk
        auto& subObject = engine.instantiate();
        subObject.transform().scale(0.004f);
        subObject.transform().setTranslation({2, 0, 2});
        subObject.transform().setRotation(glm::quat({0, glm::radians(90.0f), 0}));
        subObject.addComponent<MeshRenderer>(deskMesh, shaderProgram);
        subObject.setParent(object);
    }
    return object;
}

MapController::MapController(Object& object): Component(object)
{
}

auto MapController::onUpdate(Engine& engine) -> void
{
    if (m_segmentsPool.empty()) // TODO Initialize in something like onStart
    {
        for (int i = 0; i < 8; ++i)
        {
            auto& segment = instantiatePlaneTwoTables(engine);
            segment.setActive(false);
            m_segmentsPool.push(segment);
        }
    }

    constexpr float maxSpeedFromBase = MaxSpeed - BaseSpeed;

    auto speedCurvePosition = engine.frameInfo().time / (m_startTime + TimeToReachMaxSpeed);
    if (speedCurvePosition > 1)
        speedCurvePosition = 1;

    const float deltaTime = engine.frameInfo().deltaTime.count();
    const auto speed = BaseSpeed + (easeOutQuad(speedCurvePosition) * maxSpeedFromBase);

    if (!m_movingSegments.empty())
    {
        for (auto segment : m_movingSegments)
            segment.get().transform().translate(glm::vec3{0, 0, -speed * deltaTime});

        auto& front = m_movingSegments.front().get();
        if (front.transform().translation().z < -TMPSegmentSize)
        {
            m_movingSegments.pop_front();
            m_segmentsPool.push(front);
        }
    }

    while (m_movingSegments.size() < MinMovingSegments)
    {
        assert(!m_segmentsPool.empty() && "Must have at least MinMovingSegments available");
        auto& segment = m_segmentsPool.front().get();
        m_segmentsPool.pop();
        segment.setActive(true);

        if (m_movingSegments.empty())
        {
            segment.transform().setTranslation({0, 0, 5});
        }
        else
        {
            auto& last = m_movingSegments.back().get();
            segment.transform().setTranslation(last.transform().translation() + glm::vec3{0, 0, TMPSegmentSize});
        }
        m_movingSegments.push_back(segment);
    }
}
