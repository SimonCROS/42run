//
// Created by Simon Cros on 3/1/25.
//

#include "glad/gl.h"
#include "glm/glm.hpp"
#include <cstdlib>

#include "42runConfig.h"

import std;
import Components;
import Engine;
import InterfaceBlocks;
import Window;
import OpenGL;
import OpenGL.StateCache;
import OpenGL.Image;
import OpenGL.Texture2D2;

// class Rotator : public Component
// {
// public:
//     explicit Rotator(Object& object) : Component(object) {  }
//
//     auto onUpdate(Engine& engine) -> void override
//     {
//         object().transform().rotate(glm::quat(glm::vec3(0.0f, glm::radians(1.0f), 0.0f)));
//     }
// };

auto start() -> std::expected<void, std::string>
{
    std::cout << "42run " << FTRUN_VERSION_MAJOR << "." << FTRUN_VERSION_MINOR << std::endl;

    auto e_window_context = WindowContext::Create(4, 1);
    if (!e_window_context)
        return std::unexpected("Failed to create window context: " + std::move(e_window_context).error());

    auto e_window = Window::Create(WIDTH, HEIGHT, "42run");
    if (!e_window)
        return std::unexpected("Failed to create window: " + std::move(e_window).error());

    auto stateCache = std::make_shared<OpenGL::StateCache>();
    auto engine = Engine::Create(*std::move(e_window));

    // auto texture = OpenGL::Texture2D2(nullptr);
    // {
    //     auto e_image = OpenGL::Image::create(RESOURCE_PATH"textures/skybox/back.jpg");
    //     if (!e_image) return std::expected<void, std::string>(std::unexpect, std::move(e_image).error());
    //
    //     auto e_texture = OpenGL::Texture2D2::builder(stateCache.get())
    //                      .fromImage(*e_image, GL_RGB)
    //                      .build();
    //     if (!e_texture) return std::expected<void, std::string>(std::unexpect, std::move(e_texture).error());
    //     texture = *std::move(e_texture);
    // }

    // std::println("{}", texture);
    //
    // return {};

    auto e_shader = engine.makeShaderVariants("default",
                                              RESOURCE_PATH"shaders/default.vert",
                                              RESOURCE_PATH"shaders/default.frag");
    if (!e_shader)
        return std::unexpected(std::move(e_shader).error());

    auto e_hdrShader = engine.makeShaderVariants("hdr",
                                                 RESOURCE_PATH"shaders/hdr.vert",
                                                 RESOURCE_PATH"shaders/hdr.frag");
    if (!e_hdrShader)
        return std::unexpected(std::move(e_hdrShader).error());

    {
        auto e_return = e_hdrShader->get().enableVariant(ShaderHasNone);
        if (!e_return)
            return std::unexpected(std::move(e_return).error());
    }

    auto e_skyboxShader = engine.makeShaderVariants("skybox",
                                                    RESOURCE_PATH"shaders/skybox.vert",
                                                    RESOURCE_PATH"shaders/skybox.frag");
    if (!e_skyboxShader)
        return std::unexpected(std::move(e_skyboxShader).error());

    auto e_spheresMesh = engine.loadModel("spheres", RESOURCE_PATH"models/spheres_smooth.glb", true);
    if (!e_spheresMesh)
        return std::unexpected("Failed to load model: " + std::move(e_spheresMesh).error());

    const std::vector<std::string> faces
    {
        RESOURCE_PATH"textures/skybox/posx.jpg",
        RESOURCE_PATH"textures/skybox/negx.jpg",
        RESOURCE_PATH"textures/skybox/posy.jpg",
        RESOURCE_PATH"textures/skybox/negy.jpg",
        RESOURCE_PATH"textures/skybox/posz.jpg",
        RESOURCE_PATH"textures/skybox/negz.jpg",
    };

    auto cubemapTexture = Cubemap::Create(faces);
    {
        auto& object = engine.instantiate();
        auto e_variant = e_skyboxShader->get().enableVariant(ShaderHasNone);
        object.addComponent<SkyboxRenderer>(engine, cubemapTexture, *e_variant);
    }

    // {
    //     auto& object = engine.instantiate();
    //     object.addComponent<MeshRenderer>(*e_spheresMesh, *e_shader, cubemapTexture);
    // }
    //
    // {
    //     // Camera
    //     auto& object = engine.instantiate();
    //     object.transform().setTranslation({14.5, 15, -6.6});
    //     object.transform().setRotation(glm::quat(glm::vec3(glm::radians(-45.0f), glm::radians(115.0f), 0)));
    //
    //     const auto& camera = object.addComponent<Camera>(WIDTH, HEIGHT, 60);
    //     engine.setCamera(camera);
    //     // object.addComponent<Rotator>();
    // }

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
    map.addComponent<MapController>(cubemapTexture);

    {
        // Ancient
        auto& object = engine.instantiate();
        object.transform().scale(0.65f);
        auto& animator = object.addComponent<Animator>(*e_ancientMesh);
        auto& meshRenderer = object.addComponent<MeshRenderer>(*e_ancientMesh, *e_shader, cubemapTexture);
        object.addComponent<PlayerController>();
        meshRenderer.setAnimator(animator);
        animator.setAnimation(0);
    }

    {
        // Camera
        auto& object = engine.instantiate();
        object.transform().setTranslation({0, 3, -3.5});
        object.transform().setRotation(glm::quat(glm::vec3(glm::radians(-15.0f), glm::radians(180.0f), 0)));

        const auto& camera = object.addComponent<Camera>(WIDTH, HEIGHT, 60);
        engine.setCamera(camera);
    }

    return engine.run();
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
