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

    auto e_frogMesh = engine.loadModel("frog", RESOURCE_PATH"models/frog_jumping/scene.gltf", false);
    if (!e_frogMesh)
        return Unexpected("Failed to load model: " + std::move(e_frogMesh).error());
    auto e_golemMesh = engine.loadModel("golem", RESOURCE_PATH"models/iron_golem/scene.gltf", false);
    if (!e_golemMesh)
        return Unexpected("Failed to load model: " + std::move(e_golemMesh).error());
    auto e_villageMesh = engine.loadModel("village", RESOURCE_PATH"models/minecraft_village/scene.gltf", false);
    if (!e_villageMesh)
        return Unexpected("Failed to load model: " + std::move(e_villageMesh).error());

    {
        // Imgui object
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
        // Frog 1
        auto& object = engine.instantiate();
        object.transform().translation.x = -2.5;
        object.transform().translation.z = 1.5;
        auto& animator = object.addComponent<Animator>(*e_frogMesh);
        auto& meshRenderer = object.addComponent<MeshRenderer>(*e_frogMesh, *e_shader);
        meshRenderer.setAnimator(animator);
        animator.setAnimation(0);
        constexpr auto windowData = ImguiWindowData{
            .s_frame_x = WIDTH - 8 - 230, .s_frame_y = 8, .s_frame_width = 230, .s_frame_height = 125
        };
        auto& interface = object.addComponent<UserInterface>("Frog 1", windowData);
        interface.addBlock<CameraTargetInterfaceBlock>(1, *cameraController, 1);
        interface.addBlock<DisplayInterfaceBlock>(10);
    }

    {
        // Frog 2
        auto& object = engine.instantiate();
        object.transform().translation.x = 1.5;
        object.transform().translation.z = 8;
        auto& animator = object.addComponent<Animator>(*e_frogMesh);
        auto& meshRenderer = object.addComponent<MeshRenderer>(*e_frogMesh, *e_shader);
        meshRenderer.setAnimator(animator);
        animator.setAnimation(0);
        constexpr auto windowData = ImguiWindowData{
            .s_frame_x = WIDTH - 8 - 230, .s_frame_y = 8 + 125 + 8, .s_frame_width = 230, .s_frame_height = 125
        };
        auto& interface = object.addComponent<UserInterface>("Frog 2", windowData);
        interface.addBlock<CameraTargetInterfaceBlock>(1, *cameraController, 1);
        interface.addBlock<DisplayInterfaceBlock>(10);
    }

    {
        // Frog 3
        auto& object = engine.instantiate();
        object.transform().translation.x = 0;
        object.transform().translation.y = 0.74;
        object.transform().translation.z = -5.5;
        auto& animator = object.addComponent<Animator>(*e_frogMesh);
        auto& meshRenderer = object.addComponent<MeshRenderer>(*e_frogMesh, *e_shader);
        meshRenderer.setAnimator(animator);
        animator.setAnimation(0);
        constexpr auto windowData = ImguiWindowData{
            .s_frame_x = WIDTH - 8 - 230, .s_frame_y = 8 + 125 + 8 + 125 + 8, .s_frame_width = 230,
            .s_frame_height = 125
        };
        auto& interface = object.addComponent<UserInterface>("Frog 3", windowData);
        interface.addBlock<CameraTargetInterfaceBlock>(1, *cameraController, 1);
        interface.addBlock<DisplayInterfaceBlock>(10);
    }

    {
        // Golem
        auto& object = engine.instantiate();
        auto& animator = object.addComponent<Animator>(*e_golemMesh);
        auto& meshRenderer = object.addComponent<MeshRenderer>(*e_golemMesh, *e_shader);
        meshRenderer.setAnimator(animator);
        animator.setAnimation(7);
        constexpr auto windowData = ImguiWindowData{
            .s_frame_x = 8, .s_frame_y = 8 + 125 + 8, .s_frame_width = 230, .s_frame_height = 400
        };
        auto& interface = object.addComponent<UserInterface>("Golem", windowData);
        interface.addBlock<CameraTargetInterfaceBlock>(1, *cameraController, 5);
        interface.addBlock<DisplayInterfaceBlock>(10);
        interface.addBlock<GolemInterfaceBlock>(100);
    }

    {
        // Village
        auto& object = engine.instantiate();
        object.addComponent<MeshRenderer>(*e_villageMesh, *e_shader);
        object.transform().translation = glm::vec3(-4.2, 8.11, -4);
        object.transform().scale = glm::vec3(1.5f);
        constexpr auto windowData = ImguiWindowData{
            .s_frame_x = 8, .s_frame_y = 8, .s_frame_width = 230,
            .s_frame_height = 125
        };
        auto& interface = object.addComponent<UserInterface>("Village", windowData);
        interface.addBlock<CameraTargetInterfaceBlock>(1, *cameraController, 20);
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
