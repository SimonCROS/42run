//
// Created by loumarti on 1/25/25.
//

module;

#include <string_view>

#include "imgui_impl_opengl3.h"

module Components;

import Engine;

UserInterface::UserInterface(Object& object, const std::string_view& name, const ImguiWindowData& windowData)
    : Component(object), m_windowData(windowData)
{
    m_name = name;
}

auto UserInterface::onUpdate(Engine& engine) -> void
{
    const auto windowSize = ImVec2(static_cast<float>(m_windowData.s_frame_width),
                                   static_cast<float>(m_windowData.s_frame_height));
    const auto windowPos = ImVec2(static_cast<float>(m_windowData.s_frame_x),
                                  static_cast<float>(m_windowData.s_frame_y));
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);

    ImGui::Begin(m_name.c_str());
    uint16_t index = 0;
    for (const auto& entry : m_blocks)
    {
        entry.value->onDrawUI(index, engine, *this);
        if (index < m_blocks.size() - 1)
            InterfaceBlock::addSeparator();
        ++index;
    }
    ImGui::End();
}
