#include <fstream>
#include <iostream>

#include "GLFW/glfw3.h"

#include "42runConfig.h"
#include "Engine/Engine.h"
#include "Window/Window.h"
#include "WindowContext.h"
#include "Components/UserInterface.h"
#include "Components/CameraController.h"
#include "Components/ImguiSingleton.h"
#include "Components/MeshRenderer.h"
#include "InterfaceBlocks/CameraTargetInterfaceBlock.h"
#include "InterfaceBlocks/DisplayInterfaceBlock.h"
#include "InterfaceBlocks/GolemInterfaceBlock.h"

auto start() -> Expected<void, std::string>
{
    std::cout << "42run " << FTRUN_VERSION_MAJOR << "." << FTRUN_VERSION_MINOR << std::endl;

    auto e_window_context = WindowContext::Create(4, 1);
    if (!e_window_context)
        return Unexpected("Failed to create window context: " + std::move(e_window_context).error());

    auto e_window = Window::Create(WIDTH, HEIGHT, "42run");
    if (!e_window)
        return Unexpected("Failed to create window: " + std::move(e_window).error());

    auto engine = Engine::Create(*std::move(e_window));

    auto e_shader = engine.makeShaderVariants("default",
                                              RESOURCE_PATH"shaders/default.vert",
                                              RESOURCE_PATH"shaders/default.frag");
    if (!e_shader)
        return Unexpected(std::move(e_shader).error());

    auto e_droneMesh = engine.loadModel("drone", RESOURCE_PATH"models/buster_drone.glb", true);
    if (!e_droneMesh)
        return Unexpected("Failed to load model: " + std::move(e_droneMesh).error());

    {
        // Imgui singleton
        auto& object = engine.instantiate();
        object.addComponent<ImguiSingleton>(engine.getWindow());
    }

    CameraController* cameraController;
    {
        // Camera
        auto& object = engine.instantiate();
        const auto& camera = object.addComponent<Camera>(WIDTH, HEIGHT, 60);
        cameraController = &object.addComponent<CameraController>(glm::vec3{0, 1.4, 0}, 5);
        engine.setCamera(camera);
    }

    {
        // Drone 1
        auto& object = engine.instantiate();
        auto& animator = object.addComponent<Animator>(*e_droneMesh);
        auto& meshRenderer = object.addComponent<MeshRenderer>(*e_droneMesh, *e_shader);
        meshRenderer.setAnimator(animator);
        animator.setAnimation(0);
        constexpr auto windowData = ImguiWindowData{
            .s_frame_x = WIDTH - 8 - 230, .s_frame_y = 8, .s_frame_width = 230, .s_frame_height = 125
        };
        auto& interface = object.addComponent<UserInterface>("Buster Drone", windowData);
        interface.addBlock<CameraTargetInterfaceBlock>(1, *cameraController, 1);
        interface.addBlock<DisplayInterfaceBlock>(10);
    }

    engine.run();

    return {};
}

auto main() -> int
{
    auto e_result = start();
    if (!e_result)
    {
        std::cerr << "Error: " << e_result.error() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
