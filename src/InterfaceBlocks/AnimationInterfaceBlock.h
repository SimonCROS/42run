//
// Created by Simon Cros on 08/02/2025.
//

#ifndef GOLEMINTERFACEBLOCK_H
#define GOLEMINTERFACEBLOCK_H
#include "Components/Animator.h"
#include "Components/MeshRenderer.h"
#include "Components/UserInterface.h"

class AnimationInterfaceBlock : public InterfaceBlock
{
private:
    Animator *m_animator;

    std::vector<const char *> m_animationsNames;

public:
    explicit AnimationInterfaceBlock(UserInterface& interface);

    auto onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void override;
};

#endif //GOLEMINTERFACEBLOCK_H
