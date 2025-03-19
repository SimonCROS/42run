//
// Created by Simon Cros on 1/15/25.
//

module;

#include "GLFW/glfw3.h"

module Window;
import std;

auto WindowContext::Create(const int glMajor, const int glMinor) -> std::expected<WindowContext, std::string>
{
    if (glfwInit() == GLFW_FALSE)
    {
        const char* description;
        glfwGetError(&description);

        return std::unexpected(std::string("Failed to initialize glfw: ") + description);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    return WindowContext();
}
