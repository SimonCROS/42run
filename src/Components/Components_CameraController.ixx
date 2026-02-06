//
// Created by Simon Cros on 1/16/25.
//

module;
#include "GLFW/glfw3.h"

export module Components:CameraController;
import std;
import glm;
import Engine;
import Window;

export class CameraController final : public Component
{
public:
    static constexpr float DefaultDistance = 10.0f;

private:
    glm::vec3 m_target{};
    float m_distance{DefaultDistance};

    float m_pitch{};
    float m_yaw{};

    bool m_lastUpdateReloadShader = false;

public:
    CameraController(Object & object, const glm::vec3 target, const float distance) : Component(object),
        m_target(target), m_distance(distance)
    {}

    auto setTarget(const glm::vec3 target, const float distance = DefaultDistance) -> void
    {
        m_target = target;
        m_distance = distance;
        m_pitch = m_yaw = 0.0f;
    }

    auto onUpdate(Engine & engine) -> void override
    {
        const Controls controls = engine.controls();
        const float delta = engine.frameInfo().realDeltaTime.count();

        if (controls.isPressed(GLFW_KEY_P) && !m_lastUpdateReloadShader)
        {
            m_lastUpdateReloadShader = true;
            if (const auto && e_result = engine.getShaderManager().reloadAllShaders(); !e_result)
            {
                std::println("{}", e_result.error());
            }
        }
        else
        {
            m_lastUpdateReloadShader = false;
        }

        if (controls.isPressed(GLFW_KEY_A))
            m_yaw += delta;
        if (controls.isPressed(GLFW_KEY_D))
            m_yaw -= delta;
        if (controls.isPressed(GLFW_KEY_W))
            m_pitch += delta;
        if (controls.isPressed(GLFW_KEY_S))
            m_pitch -= delta;
        if (controls.isPressed(GLFW_KEY_Q))
            m_distance += delta * 3.0f;
        if (controls.isPressed(GLFW_KEY_E))
            m_distance -= delta * 3.0f;
        if (controls.isPressed(GLFW_KEY_R))
        {
            m_pitch = 0;
            m_yaw = 0;
            m_distance = 5;
        }
        m_distance = std::clamp(m_distance, 0.1f, 100.0f);

        const auto rotation = glm::quat(glm::vec3(m_pitch, m_yaw, 0.0f));

        const glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        const glm::vec3 position = m_target - forward * m_distance;

        object().transform().setTranslation(position);
        object().transform().setRotation(rotation);
    }
};
