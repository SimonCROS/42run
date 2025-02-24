//
// Created by Simon Cros on 1/13/25.
//

// module;
//
// #include <string>
// #include <functional>
// #include <utility>
//
// #include "GLFW/glfw3.h"
// #include "Controls.h"
// #include "Utility/Expected.h"
//
// module Window;

#include "Window_Window.h"

auto Window::Create(const int width, const int height, const std::string& title) -> Expected<Window, std::string>
{
    GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (window == nullptr)
    {
        const char* description;
        const int code = glfwGetError(&description);

        if (code != GLFW_NO_ERROR)
        {
            return Unexpected("Error while creating the window: " + std::string(description));
        }
        return Unexpected("Error while creating the window: Unknown error.");
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    return Expected<Window, std::string>(std::in_place,
                                         window,
                                         static_cast<uint32_t>(width),
                                         static_cast<uint32_t>(height));
}

Window::Window(GLFWwindow* glfwWindow, uint32_t width, uint32_t height) noexcept: m_window(glfwWindow),
    m_width(width), m_height(height)
{
    glfwSetWindowUserPointer(glfwWindow, this);
    glfwSetKeyCallback(glfwWindow, &Window::glfwKeyListener);
}

Window::Window(Window&& other) noexcept: m_window(std::exchange(other.m_window, nullptr)),
                                         m_keyListener(std::exchange(other.m_keyListener, nullptr)),
                                         m_width(std::exchange(other.m_width, {})),
                                         m_height(std::exchange(other.m_height, {}))
{
    glfwSetWindowUserPointer(m_window, this);
}

Window::~Window()
{
    if (m_window != nullptr)
    {
        glfwDestroyWindow(m_window);
    }
}

auto Window::operator=(Window&& other) noexcept -> Window&
{
    std::swap(m_window, other.m_window);
    std::swap(m_keyListener, other.m_keyListener);
    std::swap(m_width, other.m_width);
    std::swap(m_height, other.m_height);
    glfwSetWindowUserPointer(m_window, this);
    return *this;
}

auto Window::asCurrentContext(const std::function<void()>& func) const -> void
{
    const auto prev = glfwGetCurrentContext();
    if (prev == m_window)
    {
        std::invoke(func);
    }
    else
    {
        glfwMakeContextCurrent(m_window);
        std::invoke(func);
        glfwMakeContextCurrent(prev);
    }
}

auto Window::update() const -> bool
{
    if (shouldClose())
        return false;
    glfwPollEvents();
    return true;
}
