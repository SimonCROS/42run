//
// Created by Simon Cros on 08/02/2025.
//

#ifndef CAMERATARGETINTERFACEBLOCK_H
#define CAMERATARGETINTERFACEBLOCK_H
#include "Components/CameraController.h"
#include "Components/MeshRenderer.h"

#include "Components/UserInterface.h"

class CameraTargetInterfaceBlock : public InterfaceBlock
{
private:
    CameraController& m_cameraController;
    float m_focusDistance;

public:
    explicit CameraTargetInterfaceBlock(UserInterface& interface, CameraController &cameraController, float focusDistance);

    auto onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void override;
};

#endif //CAMERATARGETINTERFACEBLOCK_H
