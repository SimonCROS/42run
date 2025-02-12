//
// Created by Simon Cros on 1/15/25.
//

#ifndef WINDOWCONTEXT_H
#define WINDOWCONTEXT_H
#include <string>
#include <utility>
#include <GLFW/glfw3.h>

#include "Expected.h"

class WindowContext
{
private:
    bool m_moved = false;

    WindowContext() = default;

public:
    WindowContext(const WindowContext&) = delete;

    WindowContext(WindowContext&& other) noexcept : m_moved(std::exchange(other.m_moved, true))
    {
    }

    ~WindowContext()
    {
        if (!m_moved)
        {
            glfwTerminate();
        }
    }

    [[nodiscard]] static auto Create(int glMajor, int glMinor) -> Expected<WindowContext, std::string>;
};

#endif //WINDOWCONTEXT_H
