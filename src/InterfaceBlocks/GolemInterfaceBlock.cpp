//
// Created by Simon Cros on 08/02/2025.
//

#include "GolemInterfaceBlock.h"

#include "Engine/Object.h"

constexpr const char* parts[] = {
    "All",
    "Head",
    "Right arm", "Right arm lower", "Right hand", "Flower",
    "Left arm", "Left arm lower", "Left hand",
    "Left leg", "Left leg lower", "Left feet",
    "Right leg", "Right leg lower", "Right feet",
    "Custom",
};
constexpr int partToIndex[] = {
    1,
    17,
    68, 90, 95, 101,
    28, 48, 63,
    107, 116, 121,
    128, 135, 138,
    0,
};

constexpr int customPartIndex = IM_ARRAYSIZE(parts) - 1;

GolemInterfaceBlock::GolemInterfaceBlock(UserInterface& interface)
{
    m_animator = &interface.object().getComponent<Animator>()->get();
    m_meshRenderer = &interface.object().getComponent<MeshRenderer>()->get();

    m_animationsNames.reserve(m_animator->mesh().model().animations.size() + 1);
    m_animationsNames.push_back("-");
    for (const auto& animation : m_animator->mesh().model().animations)
        m_animationsNames.push_back(animation.name.c_str());

    m_selectedIndex = partToIndex[m_selectedPart];
}

auto GolemInterfaceBlock::onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void
{
    // Add 1 because index -1 is 0 for imgui
    int selectedIndex = m_animator->currentAnimationIndex() + 1;

    ImGui::Text("Select animation");

    if (ImGui::Combo("##animation", &selectedIndex, m_animationsNames.data(), static_cast<int>(m_animationsNames.size())))
        m_animator->setAnimation(selectedIndex - 1);

    addSeparator();

    ImGui::Text("Select golem part");
    if (ImGui::Combo("##part", &m_selectedPart, parts, IM_ARRAYSIZE(parts)))
        m_selectedIndex = partToIndex[m_selectedPart];

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
    if (m_selectedPart != customPartIndex)
        flags |= ImGuiInputTextFlags_ReadOnly;

    if (ImGui::InputInt("##mesh_index", &m_selectedIndex, 1, 10, flags))
        m_selectedIndex = std::clamp(m_selectedIndex, 0,
                                     static_cast<int>(m_meshRenderer->mesh().model().nodes.size()));

    ImGui::Text("Customize scale");

    glm::vec3 scaleMultiplier = m_meshRenderer->getScaleMultiplier(m_selectedIndex);
    if (ImGui::InputFloat("x", &scaleMultiplier.x, 0.1f, 0.5f, "%.1f"))
    {
        if (scaleMultiplier.x < 0.0f)
            scaleMultiplier.x = 0.0f;
    }
    if (ImGui::InputFloat("y", &scaleMultiplier.y, 0.1f, 0.5f, "%.1f"))
    {
        if (scaleMultiplier.y < 0.0f)
            scaleMultiplier.y = 0.0f;
    }
    if (ImGui::InputFloat("z", &scaleMultiplier.z, 0.1f, 0.5f, "%.1f"))
    {
        if (scaleMultiplier.z < 0.0f)
            scaleMultiplier.z = 0.0f;
    }

    m_meshRenderer->setScaleMultiplier(m_selectedIndex, scaleMultiplier);
}
