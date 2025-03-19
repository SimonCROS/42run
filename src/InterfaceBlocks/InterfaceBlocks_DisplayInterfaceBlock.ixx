//
// Created by Simon Cros on 08/02/2025.
//

module;

#include "imgui.h"
#include "glad/gl.h"

export module InterfaceBlocks:DisplayInterfaceBlock;
import std;
import Components;
import Engine;

constexpr const char* displayModes[] = {
    "Fill", "Line", "Point",
};
constexpr int displayModeToPolygonMode[] = {
    GL_FILL, GL_LINE, GL_POINT,
};

export class DisplayInterfaceBlock : public InterfaceBlock
{
private:
    MeshRenderer* m_meshRenderer;

    int m_selectedDisplayMode{0};
    bool m_displayed{true};

public:
    explicit DisplayInterfaceBlock(UserInterface& interface)
    {
        m_meshRenderer = &interface.object().getComponent<MeshRenderer>()->get();
    }

    auto onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void override
    {
        ImGui::Text("Display");

        if (ImGui::Checkbox("##displayed", &m_displayed))
            m_meshRenderer->setDisplay(m_displayed);

        ImGui::SameLine();

        if (ImGui::Combo("##display mode", &m_selectedDisplayMode, displayModes, IM_ARRAYSIZE(displayModes)))
            m_meshRenderer->setPolygoneMode(displayModeToPolygonMode[m_selectedDisplayMode]);
    }
};
