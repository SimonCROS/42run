//
// Created by Simon Cros on 1/16/25.
//

module;

#include "GLFW/glfw3.h"

export module Window:Controls;

export class Controls
{
private:
    GLFWwindow* m_window;

public:
    explicit Controls(GLFWwindow* window): m_window(window)
    {
    }

    [[nodiscard]] auto isShiftPressed() const -> bool
    {
        return glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    }

    [[nodiscard]] auto isAltPressed() const -> bool
    {
        return glfwGetKey(m_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
    }

    [[nodiscard]] auto isPressed(const int key) const -> bool
    {
        return glfwGetKey(m_window, key);
    }
};
