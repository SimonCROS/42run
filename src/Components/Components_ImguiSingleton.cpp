//
// Created by loumarti on 2/8/25.
//

module;

#include <iostream>
#include <ostream>
#include <cassert>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Engine/Object.h"
#include "Engine/Engine_Component.ixx"

module Components;

import Window;

ImguiSingleton::ImguiSingleton(Object& object, const Window& window)
    : Component(object)
{
    assert(!s_singletonExists);
    s_singletonExists = true;
    initializeImGui(window);
}

ImguiSingleton::~ImguiSingleton()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    s_singletonExists = false;
}

void ImguiSingleton::onWillUpdate(Engine& engine)
{
    newFrame();
}

void ImguiSingleton::onPostRender(Engine& engine)
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


auto ImguiSingleton::initializeImGui(const Window& window) -> void
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename = "lib/imgui/cache/imgui.ini";
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.getGLFWHandle(), true);
    ImGui_ImplOpenGL3_Init();
}

auto ImguiSingleton::newFrame() const -> void
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
