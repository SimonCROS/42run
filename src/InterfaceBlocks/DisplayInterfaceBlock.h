//
// Created by Simon Cros on 08/02/2025.
//

#ifndef DISPLAYINTERFACEBLOCK_H
#define DISPLAYINTERFACEBLOCK_H
#include "Components/MeshRenderer.h"
#include "Components/UserInterface.h"

class DisplayInterfaceBlock : public InterfaceBlock
{
private:
    MeshRenderer *m_meshRenderer;

    int m_selectedDisplayMode{0};
    bool m_displayed{true};

public:
    explicit DisplayInterfaceBlock(UserInterface& interface);

    auto onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void override;
};

#endif //DISPLAYINTERFACEBLOCK_H
