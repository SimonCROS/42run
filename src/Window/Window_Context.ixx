//
// Created by Simon Cros on 2/24/25.
//

module;

#include <string>
#include <utility>

#include "GLFW/glfw3.h"
#include "Utility/Expected.h"

export module Window:Context;

export class WindowContext
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
