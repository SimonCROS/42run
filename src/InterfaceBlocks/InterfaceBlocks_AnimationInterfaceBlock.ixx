//
// Created by Simon Cros on 08/02/2025.
//

module;

#include <vector>
#include <cstdint>

export module InterfaceBlocks:AnimationInterfaceBlock;

import Components;
import Engine;

export class AnimationInterfaceBlock : public InterfaceBlock
{
private:
    Animator *m_animator;

    std::vector<const char *> m_animationsNames;

public:
    explicit AnimationInterfaceBlock(UserInterface& interface);

    auto onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void override;
};
