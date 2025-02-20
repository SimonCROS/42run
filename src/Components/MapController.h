//
// Created by Simon Cros on 2/20/25.
//

#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H
#include "Engine/Engine.h"
#include "Engine/EngineComponent.h"

class MapController final : public EngineComponent
{
private:
    static constexpr float LaneSize = 1;
    static constexpr float BaseSpeed = 2;
    static constexpr float MaxSpeed = 20;
    static constexpr DurationType TimeToReachMaxSpeed = std::chrono::duration_cast<DurationType>(
        std::chrono::seconds(120));

    bool m_isLeftPressed{false};
    bool m_isRightPressed{false};

    DurationType m_startTime{}; // TODO set in something like onStart

    static constexpr auto easeOutQuad(const float x) -> float
    {
        return 1 - (1 - x) * (1 - x);
    }

public:
    explicit MapController(Object& object) : EngineComponent(object)
    {
    }

    auto onUpdate(Engine& engine) -> void override
    {
        constexpr float maxSpeedFromBase = MaxSpeed - BaseSpeed;

        auto speedCurvePosition = engine.frameInfo().time / (m_startTime + TimeToReachMaxSpeed);
        if (speedCurvePosition > 1)
            speedCurvePosition = 1;

        const float deltaTime = engine.frameInfo().deltaTime.count();
        const auto speed = BaseSpeed + (easeOutQuad(speedCurvePosition) * maxSpeedFromBase);

        object().transform().setTranslation(object().transform().translation() + glm::vec3{0, 0, -speed * deltaTime});
    }
};

#endif //MAPCONTROLLER_H
