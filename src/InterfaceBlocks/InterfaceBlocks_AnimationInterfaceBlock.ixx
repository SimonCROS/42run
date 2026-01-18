//
// Created by Simon Cros on 08/02/2025.
//

module;

#include "imgui.h"

export module InterfaceBlocks:AnimationInterfaceBlock;
import std.compat;
import Components;
import Engine;

export class AnimationInterfaceBlock : public InterfaceBlock
{
private:
    Animator* m_animator;

    std::vector<const char*> m_animationsNames;

public:
    explicit AnimationInterfaceBlock(UserInterface& interface)
    {
        m_animator = &interface.object().getComponent<Animator>()->get();

        m_animationsNames.reserve(m_animator->animations().size() + 1);
        m_animationsNames.push_back("-");
        for (const auto& animation : m_animator->animations())
            m_animationsNames.push_back(animation.name().c_str());
    }

    auto onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void override
    {
        // Add 1 because index -1 is 0 for imgui
        int selectedIndex = m_animator->currentAnimationIndex() + 1;

        ImGui::Text("Select animation");

        if (ImGui::Combo("##animation", &selectedIndex, m_animationsNames.data(),
                         static_cast<int>(m_animationsNames.size())))
            m_animator->setAnimation(selectedIndex - 1);
    }
};
