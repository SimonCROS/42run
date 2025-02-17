#include <fstream>
#include <iostream>

#include "42runConfig.h"
#include "Engine/Engine.h"
#include "Window/Window.h"
#include "Window/WindowContext.h"
#include "Components/UserInterface.h"
#include "Components/CameraController.h"
#include "Components/ImguiSingleton.h"
#include "Components/MeshRenderer.h"
#include "InterfaceBlocks/CameraTargetInterfaceBlock.h"
#include "InterfaceBlocks/DisplayInterfaceBlock.h"
#include "InterfaceBlocks/AnimationInterfaceBlock.h"

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

    auto e_floorMesh = engine.loadModel("floor", RESOURCE_PATH"models/floor.glb", true);
    if (!e_floorMesh)
        return Unexpected("Failed to load model: " + std::move(e_floorMesh).error());

    auto e_deskMesh = engine.loadModel("desk", RESOURCE_PATH"models/desk.glb", true);
    if (!e_deskMesh)
        return Unexpected("Failed to load model: " + std::move(e_deskMesh).error());

    auto e_ancientMesh = engine.loadModel("ancient", RESOURCE_PATH"models/ancient.glb", true);
    if (!e_ancientMesh)
        return Unexpected("Failed to load model: " + std::move(e_ancientMesh).error());

    {
        // Floor
        auto& object = engine.instantiate();
        object.transform().translation = {0, 0, 0};
        object.addComponent<MeshRenderer>(*e_floorMesh, *e_shader);
    }

    {
        // Desk
        auto& object = engine.instantiate();
        object.transform().scale *= 0.005f;
        object.transform().translation = {-2, 0, -5};
        object.transform().rotation = glm::quat({0, glm::radians(-90.0f), 0});
        object.addComponent<MeshRenderer>(*e_deskMesh, *e_shader);
    }

    {
        // Desk
        auto& object = engine.instantiate();
        object.transform().scale *= 0.005f;
        object.transform().translation = {2, 0, -2};
        object.transform().rotation = glm::quat({0, glm::radians(-90.0f), 0});
        object.addComponent<MeshRenderer>(*e_deskMesh, *e_shader);
    }

    {
        // Ancient
        auto& object = engine.instantiate();
        auto& animator = object.addComponent<Animator>(*e_ancientMesh);
        auto& meshRenderer = object.addComponent<MeshRenderer>(*e_ancientMesh, *e_shader);
        meshRenderer.setAnimator(animator);
        animator.setAnimation(0);
        constexpr auto windowData = ImguiWindowData{
            .s_frame_x = WIDTH - 8 - 230, .s_frame_y = 8, .s_frame_width = 230, .s_frame_height = 200
        };
        auto& interface = object.addComponent<UserInterface>("Ancient", windowData);
        interface.addBlock<CameraTargetInterfaceBlock>(1, *cameraController, 1);
        interface.addBlock<DisplayInterfaceBlock>(10);
        interface.addBlock<AnimationInterfaceBlock>(100);
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
