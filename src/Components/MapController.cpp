//
// Created by Simon Cros on 2/20/25.
//

#include "MapController.h"

void MapController::onUpdate(Engine& engine)
{
    constexpr float maxSpeedFromBase = MaxSpeed - BaseSpeed;

    auto speedCurvePosition = engine.frameInfo().time / (m_startTime + TimeToReachMaxSpeed);
    if (speedCurvePosition > 1)
        speedCurvePosition = 1;

    const float deltaTime = engine.frameInfo().deltaTime.count();
    const auto speed = BaseSpeed + (easeOutQuad(speedCurvePosition) * maxSpeedFromBase);

    object().transform().setTranslation(object().transform().translation() + glm::vec3{0, 0, -speed * deltaTime});
}
