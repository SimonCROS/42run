//
// Created by Simon Cros on 2/20/25.
//

module;

export module Components:MapController;

import Engine;

export class MapController final : public Component
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
    explicit MapController(Object& object) : Component(object)
    {
    }

    auto onUpdate(Engine& engine) -> void override;
};
