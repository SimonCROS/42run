//
// Created by Simon Cros on 08/02/2025.
//

module;

#include <cstdint>

export module InterfaceBlocks:DisplayInterfaceBlock;

import Components;
import Engine;

export class DisplayInterfaceBlock : public InterfaceBlock
{
private:
    MeshRenderer *m_meshRenderer;

    int m_selectedDisplayMode{0};
    bool m_displayed{true};

public:
    explicit DisplayInterfaceBlock(UserInterface& interface);

    auto onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void override;
};
