//
// Created by Simon Cros on 2/20/25.
//

#include "PlayerController.h"

auto PlayerController::ChangeLaneAnimation::update(const float deltaTime) -> void
{
    if (isOver())
        return;

    m_t += deltaTime / MoveDuration.count();
    m_t = std::min(m_t, 1.0f);

    m_object.transform().setTranslation({std::lerp(m_fromLanePosition, m_toLanePosition, m_t), 0, 0});
}

auto PlayerController::processInput(const Engine& engine) -> void
{
    const auto controls = engine.controls();

    const bool leftPressed = controls.isPressed(GLFW_KEY_A) || controls.isPressed(GLFW_KEY_LEFT);
    const bool rightPressed = controls.isPressed(GLFW_KEY_D) || controls.isPressed(GLFW_KEY_RIGHT);

    Move move = MoveNone;
    if (leftPressed != rightPressed)
    {
        if (leftPressed && leftPressed != m_isLeftPressed)
            move = MoveLeft;
        if (rightPressed && rightPressed != m_isRightPressed)
            move = MoveRight;
    }

    m_isLeftPressed = leftPressed;
    m_isRightPressed = rightPressed;

    if (move != MoveNone)
    {
        if (m_currentMove == MoveNone)
            m_currentMove = move;
        else
            m_nextMove = move;
    }
}
