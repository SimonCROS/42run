//
// Created by Simon Cros on 08/02/2025.
//

#include "DisplayInterfaceBlock.h"
#include "Engine/Object.h"

constexpr const char* displayModes[] = {
    "Fill", "Line", "Point",
};
constexpr int displayModeToPolygonMode[] = {
    GL_FILL, GL_LINE, GL_POINT,
};

DisplayInterfaceBlock::DisplayInterfaceBlock(UserInterface& interface)
{
    m_meshRenderer = &interface.object().getComponent<MeshRenderer>()->get();
}

auto DisplayInterfaceBlock::onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void
{
    ImGui::Text("Display");

    if (ImGui::Checkbox("##displayed", &m_displayed))
        m_meshRenderer->setDisplay(m_displayed);

    ImGui::SameLine();

    if (ImGui::Combo("##display mode", &m_selectedDisplayMode, displayModes, IM_ARRAYSIZE(displayModes)))
        m_meshRenderer->setPolygoneMode(engine, displayModeToPolygonMode[m_selectedDisplayMode]);
}
