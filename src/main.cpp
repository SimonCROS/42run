//
// Created by Simon Cros on 3/1/25.
//

#include <fstream>
#include <memory>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "42runConfig.h"
#include <expected>

import Components;
import Engine;
import InterfaceBlocks;
import Window;

auto start() -> std::expected<void, std::string>
{
    std::cout << "42run " << FTRUN_VERSION_MAJOR << "." << FTRUN_VERSION_MINOR << std::endl;

    auto e_window_context = WindowContext::Create(4, 1);
    if (!e_window_context)
        return std::unexpected("Failed to create window context: " + std::move(e_window_context).error());

    auto e_window = Window::Create(WIDTH, HEIGHT, "42run");
    if (!e_window)
        return std::unexpected("Failed to create window: " + std::move(e_window).error());

    auto engine = Engine::Create(*std::move(e_window));

    auto e_shader = engine.makeShaderVariants("default",
                                              RESOURCE_PATH"shaders/default.vert",
                                              RESOURCE_PATH"shaders/default.frag");
    if (!e_shader)
        return std::unexpected(std::move(e_shader).error());

    auto e_floorMesh = engine.loadModel("floor", RESOURCE_PATH"models/floor.glb", true);
    if (!e_floorMesh)
        return std::unexpected("Failed to load model: " + std::move(e_floorMesh).error());

    auto e_deskMesh = engine.loadModel("desk", RESOURCE_PATH"models/desk.glb", true);
    if (!e_deskMesh)
        return std::unexpected("Failed to load model: " + std::move(e_deskMesh).error());

    auto e_ancientMesh = engine.loadModel("ancient", RESOURCE_PATH"models/ancient.glb", true);
    if (!e_ancientMesh)
        return std::unexpected("Failed to load model: " + std::move(e_ancientMesh).error());

    auto& map = engine.instantiate();
    map.addComponent<MapController>();

    {
        // Ancient
        auto& object = engine.instantiate();
        auto& animator = object.addComponent<Animator>(*e_ancientMesh);
        auto& meshRenderer = object.addComponent<MeshRenderer>(*e_ancientMesh, *e_shader);
        object.addComponent<PlayerController>();
        meshRenderer.setAnimator(animator);
        animator.setAnimation(0);
    }

    {
        // Camera
        auto& object = engine.instantiate();
        object.transform().setTranslation({0, 5, -5});
        object.transform().setRotation(glm::quat(glm::vec3(glm::radians(-20.0f), glm::radians(180.0f), 0)));

        const auto& camera = object.addComponent<Camera>(WIDTH, HEIGHT, 60);
        engine.setCamera(camera);
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
