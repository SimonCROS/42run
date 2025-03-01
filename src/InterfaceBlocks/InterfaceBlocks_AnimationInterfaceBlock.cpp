//
// Created by Simon Cros on 08/02/2025.
//

module;

#include <vector>
#include <cstdint>

#include "imgui.h"

module InterfaceBlocks;
import Components;
import Engine;

AnimationInterfaceBlock::AnimationInterfaceBlock(UserInterface& interface)
{
    m_animator = &interface.object().getComponent<Animator>()->get();

    m_animationsNames.reserve(m_animator->mesh().model().animations.size() + 1);
    m_animationsNames.push_back("-");
    for (const auto& animation : m_animator->mesh().model().animations)
        m_animationsNames.push_back(animation.name.c_str());
}

auto AnimationInterfaceBlock::onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void
{
    // Add 1 because index -1 is 0 for imgui
    int selectedIndex = m_animator->currentAnimationIndex() + 1;

    ImGui::Text("Select animation");

    if (ImGui::Combo("##animation", &selectedIndex, m_animationsNames.data(), static_cast<int>(m_animationsNames.size())))
        m_animator->setAnimation(selectedIndex - 1);
}
