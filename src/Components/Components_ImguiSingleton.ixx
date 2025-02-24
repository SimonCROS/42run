//
// Created by loumarti on 2/8/25.
//

module;

#include "Engine/EngineComponent.h"

export module Components:ImguiSingleton;

import Window;

export class ImguiSingleton final : public EngineComponent {
private:
    inline static bool s_singletonExists{false};

public:
    explicit ImguiSingleton(Object& object, const Window& window);
    ~ImguiSingleton() override;

    ImguiSingleton(const ImguiSingleton&) = delete;
    ImguiSingleton& operator=(const ImguiSingleton&) = delete;

    auto onWillUpdate(Engine& engine) -> void override;
    auto onPostRender(Engine& engine) -> void override;

private:
    auto initializeImGui(const Window& window) -> void;
    auto newFrame() const -> void;
};
