//
// Created by Simon Cros on 2/20/25.
//

export module Components:MapController;
import std;
import Engine;

export class MapController final : public Component
{
private:
    static constexpr float BaseSpeed = 2;
    static constexpr float MaxSpeed = 20;
    static constexpr int MinMovingSegments = 4;
    static constexpr float TMPSegmentSize = 20;
    static constexpr DurationType TimeToReachMaxSpeed = std::chrono::duration_cast<DurationType>(
        std::chrono::seconds(120));

    std::queue<std::reference_wrapper<Object>> m_segmentsPool;
    std::deque<std::reference_wrapper<Object>> m_movingSegments;

    DurationType m_startTime{}; // TODO set in something like onStart

    static constexpr auto easeOutQuad(const float x) -> float
    {
        return 1 - (1 - x) * (1 - x);
    }

public:
    explicit MapController(Object& object);

    auto onUpdate(Engine& engine) -> void override;
};
