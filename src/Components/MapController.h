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
    static constexpr float BaseSpeed = 2;
    static constexpr float MaxSpeed = 20;
    static constexpr DurationType TimeToReachMaxSpeed = std::chrono::duration_cast<DurationType>(
        std::chrono::seconds(120));

    DurationType m_startTime{}; // TODO set in something like onStart

    static constexpr auto easeOutQuad(const float x) -> float
    {
        return 1 - (1 - x) * (1 - x);
    }

public:
    explicit MapController(Object& object) : EngineComponent(object)
    {
    }

    auto onUpdate(Engine& engine) -> void override;
};

#endif //MAPCONTROLLER_H
