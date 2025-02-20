//
// Created by Simon Cros on 2/20/25.
//

#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H
#include "Engine/EngineComponent.h"


class PlayerController : EngineComponent {
public:
    auto onUpdate(Engine& engine) -> void override
    {
        // const auto controls = engine.controls();
        //
        // const bool leftPressed = controls.isPressed(GLFW_KEY_A) || controls.isPressed(GLFW_KEY_LEFT);
        // const bool rightPressed = controls.isPressed(GLFW_KEY_D) || controls.isPressed(GLFW_KEY_RIGHT);
        //
        // glm::vec3 move{};
        // if (leftPressed != m_isLeftPressed)
        // {
        //     move.x -= LaneSize;
        // }
        //
        // m_isLeftPressed = leftPressed;
        // m_isRightPressed = rightPressed;
    }
};



#endif //PLAYERCONTROLLER_H
