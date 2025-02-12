//
// Created by Simon Cros on 08/02/2025.
//

#include "CameraTargetInterfaceBlock.h"

CameraTargetInterfaceBlock::CameraTargetInterfaceBlock(UserInterface& interface, CameraController& cameraController,
                                                       const float focusDistance)
    : m_cameraController(cameraController), m_focusDistance(focusDistance)
{
}

auto CameraTargetInterfaceBlock::onDrawUI(uint16_t blockId, Engine& engine, UserInterface& interface) -> void
{
    if (ImGui::Button("Focus"))
        m_cameraController.setTarget(interface.object().transform().translation, m_focusDistance);
}
