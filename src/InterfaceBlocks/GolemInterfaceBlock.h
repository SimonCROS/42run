//
// Created by Simon Cros on 08/02/2025.
//

#ifndef GOLEMINTERFACEBLOCK_H
#define GOLEMINTERFACEBLOCK_H
#include "Components/Animator.h"
#include "Components/MeshRenderer.h"
#include "Components/UserInterface.h"

class GolemInterfaceBlock : public InterfaceBlock
{
private:
    Animator *m_animator;
    MeshRenderer *m_meshRenderer;

    std::vector<const char *> m_animationsNames;

    int m_selectedPart{0};
    int m_selectedIndex{0};

public:
    explicit GolemInterfaceBlock(UserInterface& interface);

    auto onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void override;
};

#endif //GOLEMINTERFACEBLOCK_H
