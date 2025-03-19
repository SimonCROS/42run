//
// Created by Simon Cros on 1/13/25.
//

module;

#include "GLFW/glfw3.h"

export module Window:Window;
import std;

import :Controls;

export class Window
{
public:
    using KeyListener = std::function<void (Window&, int, int, int)>;

private:
    GLFWwindow* m_window;
    KeyListener m_keyListener;
    uint32_t m_width;
    uint32_t m_height;

    static auto glfwKeyListener(GLFWwindow* glfwWindow, int key, int scancode, int action, int mode) -> void
    {
        auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        if (window != nullptr && window->m_keyListener != nullptr)
        {
            std::invoke(window->m_keyListener, *window, key, action, mode);
        }
    }

public:
    [[nodiscard]] static auto Create(int width, int height, const std::string& title) -> std::expected<Window, std::string>;

    explicit Window(GLFWwindow* glfwWindow, uint32_t width, uint32_t height) noexcept;
    Window(const Window&) = delete;
    Window(Window&& other) noexcept;
    ~Window();

    auto operator=(const Window&) -> Window& = delete;
    auto operator=(Window&& other) noexcept -> Window&;

    [[nodiscard]] auto getCurrentControls() const -> Controls
    {
        return Controls(m_window);
    }

    auto setAsCurrentContext() const -> void
    {
        glfwMakeContextCurrent(m_window);
    }

    auto asCurrentContext(const std::function<void()>& func) const -> void;

    auto setKeyCallback(const KeyListener& callback) -> void
    {
        m_keyListener = callback;
    }

    auto setShouldClose() const -> void
    {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }

    [[nodiscard]] auto shouldClose() const -> bool
    {
        return glfwWindowShouldClose(m_window);
    }

    auto update() const -> bool; // NOLINT(*-use-nodiscard)

    auto swapBuffers() const -> void
    {
        glfwSwapBuffers(m_window);
    }

    [[nodiscard]] auto width() const -> uint32_t
    {
        return m_width;
    }

    [[nodiscard]] auto height() const -> uint32_t
    {
        return m_height;
    }

    // [!] temporary
    [[nodiscard]] auto getGLFWHandle() const -> GLFWwindow*
    {
        return m_window;
    }
};
