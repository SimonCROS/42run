//
// Created by Simon Cros on 2/20/25.
//

module;

#include <chrono>

export module Components:PlayerController;

import Engine;

export class PlayerController final : public Component
{
public:
    static constexpr float LaneSize = 1.2;

private:
    enum Move
    {
        MoveNone,
        MoveLeft,
        MoveRight,
    };

    class ChangeLaneAnimation
    {
        static constexpr DurationType MoveDuration = std::chrono::duration<float>(0.15f);

    private:
        float m_fromLanePosition;
        float m_toLanePosition;

        float m_t;
        Object& m_object;

    public:
        ChangeLaneAnimation(const float fromLanePosition, const float toLanePosition, Object& object)
            : m_fromLanePosition(fromLanePosition), m_toLanePosition(toLanePosition), m_t(0), m_object(object)
        {
        }

        auto update(float deltaTime) -> void;

        [[nodiscard]] auto isOver() const -> bool { return m_t >= 1.0f; };
    };

    bool m_isLeftPressed{false};
    bool m_isRightPressed{false};

    float m_lanePosition{0};

    std::optional<ChangeLaneAnimation> mo_animation{};

    Move m_currentMove{};
    Move m_nextMove{}; // Cache for next move. Overwritten when new input is received while m_isMoving is true.

public:
    explicit PlayerController(Object& object) : Component(object)
    {
    }

    auto onUpdate(Engine& engine) -> void override
    {
        processInput(engine);

        if (mo_animation.has_value())
        {
            if (mo_animation->isOver())
            {
                m_currentMove = m_nextMove;
                m_nextMove = MoveNone;
                mo_animation.reset();
            }
            else
            {
                mo_animation->update(engine.frameInfo().deltaTime.count());
            }
        }

        if (!mo_animation.has_value() && m_currentMove != MoveNone)
        {
            float fromLanePosition = m_lanePosition;
            float toLanePosition;

            if (m_currentMove == MoveLeft)
                toLanePosition = fromLanePosition + LaneSize;
            else
                toLanePosition = fromLanePosition - LaneSize;

            // clamp toLanePosition
            toLanePosition = std::clamp(toLanePosition, -LaneSize, LaneSize);

            if (fromLanePosition != toLanePosition)
            {
                m_lanePosition = toLanePosition;
                mo_animation.emplace(fromLanePosition, m_lanePosition, object());
            }
            else
            {
                m_currentMove = MoveNone;
            }
        }
    }

private:
    auto processInput(const Engine& engine) -> void;
};
