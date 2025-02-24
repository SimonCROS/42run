//
// Created by Simon Cros on 2/20/25.
//

module;

#include "MeshRenderer.h"

module Components;

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
        subObject.transform().setScale({0.005f, 0.005f, 0.005f});
        subObject.transform().setTranslation({-2, 0, 5});
        subObject.transform().setRotation(glm::quat({0, glm::radians(90.0f), 0}));
        subObject.addComponent<MeshRenderer>(deskMesh, shaderProgram);
        subObject.setParent(object);
    }

    {
        // Desk
        auto& subObject = engine.instantiate();
        subObject.transform().setScale({0.005f, 0.005f, 0.005f});
        subObject.transform().setTranslation({2, 0, 2});
        subObject.transform().setRotation(glm::quat({0, glm::radians(90.0f), 0}));
        subObject.addComponent<MeshRenderer>(deskMesh, shaderProgram);
        subObject.setParent(object);
    }
    return object;
}

auto MapController::onUpdate(Engine& engine) -> void
{
    constexpr float maxSpeedFromBase = MaxSpeed - BaseSpeed;

    auto speedCurvePosition = engine.frameInfo().time / (m_startTime + TimeToReachMaxSpeed);
    if (speedCurvePosition > 1)
        speedCurvePosition = 1;

    const float deltaTime = engine.frameInfo().deltaTime.count();
    const auto speed = BaseSpeed + (easeOutQuad(speedCurvePosition) * maxSpeedFromBase);

    object().transform().setTranslation(object().transform().translation() + glm::vec3{0, 0, -speed * deltaTime});
}
