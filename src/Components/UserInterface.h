//
// Created by loumarti on 1/25/25.
//

#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include <imgui.h>

#include "Engine/Engine.h"
#include "Engine/EngineComponent.h"
#include "Utility/VectorMultiMap.h"

class MeshRenderer;

struct ImguiWindowData
{
    int s_frame_x = 8;
    int s_frame_y = 8;
    int s_frame_width = 230;
    int s_frame_height = 132;
};

class InterfaceBlock;
class UserInterface;

class InterfaceBlock
{
private:
    static constexpr float s_text_offset = 100.0f;
    static constexpr float s_section_padding = 8.0f;

public:
    virtual ~InterfaceBlock() = default;

    virtual auto onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void = 0;

    static auto addSeparator() -> void
    {
        ImGui::Dummy(ImVec2(0, s_section_padding));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, s_section_padding));
    }
};

class UserInterface : public EngineComponent
{
protected:
    std::string m_name;
    ImguiWindowData m_windowData;

    VectorMultiMap<float, std::unique_ptr<InterfaceBlock>> m_blocks;

public:
    explicit UserInterface(Object& object, const std::string_view& name = "default interface", const ImguiWindowData& windowData = {});
    UserInterface(const UserInterface& other) = delete;
    ~UserInterface() override = default;

    template <class T, class... Args>
        requires std::derived_from<T, InterfaceBlock> && std::constructible_from<T, UserInterface&, Args...>
    auto addBlock(const float order, Args&&... args) -> void
    {
        m_blocks.emplace(order, std::make_unique<T>(*this, std::forward<Args>(args)...));
    }

    auto operator=(const UserInterface& other) -> UserInterface& = delete;
    auto onUpdate(Engine& engine) -> void override;
};

#endif //USERINTERFACE_H
