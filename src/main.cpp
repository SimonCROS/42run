//
// Created by Simon Cros on 3/1/25.
//

#include "glad/gl.h"
#include <cstdlib>

#include "42runConfig.h"
#include "stb_image.h"
#include "tiny_gltf.h"

import std;
import glm;
import Components;
import Engine;
import InterfaceBlocks;
import Window;
import Image;
import OpenGL;
import OpenGL.StateCache;
import OpenGL.Texture2D;
import OpenGL.Cubemap;
import DataCache;
import Utility.SlotSet;

class Rotator : public Component
{
private:
    glm::vec3 m_axis;

public:
    explicit Rotator(Object & object, const glm::vec3 axis) : Component(object), m_axis(axis) {}

    auto onUpdate(Engine & engine) -> void override
    {
        object().transform().rotate(glm::quat(m_axis * glm::radians(0.2f)));
    }
};

auto start() -> std::expected<void, std::string>
{
    std::cout << "42run " << FTRUN_VERSION_MAJOR << "." << FTRUN_VERSION_MINOR << std::endl;

    TRY_V(auto, windowContext, WindowContext::Create(4, 1));
    TRY_V(auto, window, Window::Create(WIDTH, HEIGHT, "42run"));

    auto stateCache = std::make_shared<OpenGL::StateCache>();
    auto engine = Engine::Create(std::move(window));

    // ********************************
    // Init shaders
    // TODO safe
    // ********************************

    TRY_V(const SlotSetIndex, defaultVertIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/pbr.vert"));
    TRY_V(const SlotSetIndex, defaultFragIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/pbr.frag"));

    *engine.getShaderManager().getOrCreateShaderProgram(
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/texcoord.vert"),
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/hdr.frag"), ShaderFlags::None);

    const auto brdfProgramIdx = *engine.getShaderManager().getOrCreateShaderProgram(
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/texcoord.vert"),
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/brdf.frag"), ShaderFlags::None);

    const auto eqProgramIdx = *engine.getShaderManager().getOrCreateShaderProgram(
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/cubemap.vert"),
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/equirectangular_to_cubemap.frag"),
        ShaderFlags::None);

    const auto irradianceProgramIdx = *engine.getShaderManager().getOrCreateShaderProgram(
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/cubemap.vert"),
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/irradiance.frag"), ShaderFlags::None);

    const auto prefilterProgramIdx = *engine.getShaderManager().getOrCreateShaderProgram(
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/cubemap.vert"),
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/prefilter.frag"), ShaderFlags::None);

    *engine.getShaderManager().getOrCreateShaderProgram(
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/skybox.vert"),
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/skybox.frag"), ShaderFlags::None);


    // ********************************
    // Init models
    // ********************************

    stbi_set_flip_vertically_on_load(false);
    TRY_V(auto, spheresMesh, engine.loadModel("spheres", RESOURCE_PATH"models/spheres.glb", true));
    TRY_V(auto, ancientMesh, engine.loadModel("ancient", RESOURCE_PATH"models/character.glb", true));
    TRY_V(auto, floorMesh, engine.loadModel("floor", RESOURCE_PATH"models/floor.glb", true));
    TRY_V(auto, deskMesh, engine.loadModel("desk", RESOURCE_PATH"models/desk.glb", true));
    stbi_set_flip_vertically_on_load(true);


    // ********************************
    // Prepare shaders for models
    // ********************************

    TRY(spheresMesh.get().prepareShaderPrograms(engine.getShaderManager(), defaultVertIdx, defaultFragIdx));
    TRY(ancientMesh.get().prepareShaderPrograms(engine.getShaderManager(), defaultVertIdx, defaultFragIdx));
    TRY(floorMesh.get().prepareShaderPrograms(engine.getShaderManager(), defaultVertIdx, defaultFragIdx));
    TRY(deskMesh.get().prepareShaderPrograms(engine.getShaderManager(), defaultVertIdx, defaultFragIdx));

    // ********************************
    // Compile and link shaders
    // ********************************

    TRY(engine.getShaderManager().reloadAllShaders());

    // ********************************
    // Prepare IBL
    // ********************************

    const GLuint cubemapSize = 512;

    TRY_V(auto, hdrTexture, DataCache::loadOrCreate<OpenGL::Texture2D>(std::filesystem::path("aaa/a.cubemap"), [&stateCache] -> std::expected<OpenGL::Texture2D, std::string>
    {
        TRY_V(auto, hdrImage, Image::Create(RESOURCE_PATH"textures/skybox/san_giuseppe_bridge_1k.hdr"));

        return OpenGL::Texture2D::builder(stateCache.get()).fromImage(hdrImage, GL_RGB32F).build();
    }));

    TRY_V(auto, skybox, OpenGL::Cubemap::builder(stateCache.get())
        .withFormat(GL_RGB32F, GL_RGB, GL_UNSIGNED_BYTE)
        .withSize(cubemapSize)
        .build());

    if (!skybox.fromCache(".cache/skybox.cubemap"))
    {
        TRY(skybox.fromEquirectangular(engine.getShaderManager().getProgram(eqProgramIdx), hdrTexture));
    }

    std::println("{}", hdrTexture);
    auto CubemapTexture = *OpenGL::Cubemap::builder(stateCache.get()).withFormat(
        hdrTexture.internalFormat(), hdrTexture.format(), hdrTexture.type()).withSize(cubemapSize).build();
    *CubemapTexture.fromEquirectangular(engine.getShaderManager().getProgram(eqProgramIdx), hdrTexture);

    auto irradianceMap = *OpenGL::Cubemap::builder(stateCache.get()).withFormat(
        hdrTexture.internalFormat(), hdrTexture.format(), hdrTexture.type()).withSize(cubemapSize).build();
    *irradianceMap.fromCubemap(engine.getShaderManager().getProgram(irradianceProgramIdx), CubemapTexture, 0);

    auto prefilterMap = *OpenGL::Cubemap::builder(stateCache.get()).withFormat(
        hdrTexture.internalFormat(), hdrTexture.format(), hdrTexture.type()).withSize(cubemapSize).withFiltering(
        GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR).build();
    *prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), CubemapTexture, 0);
    *prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), CubemapTexture, 1);
    *prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), CubemapTexture, 2);
    *prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), CubemapTexture, 3);
    *prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), CubemapTexture, 4);

    auto brdfTexture = *OpenGL::Texture2D::builder(stateCache.get()).withDimensions(cubemapSize, cubemapSize).
            withFormat(GL_RG16F, GL_RG, hdrTexture.type()).build();
    *brdfTexture.fromShader(engine.getShaderManager().getProgram(brdfProgramIdx));


    // ********************************
    // Manually create the scene
    // ********************************

    {
        auto & object = engine.instantiate();
        object.addComponent<ImguiSingleton>(engine.getWindow());
    }

    {
        auto & object = engine.instantiate();
        object.addComponent<SkyboxRenderer>(engine, irradianceMap);
    }

    // {
    //     auto & object = engine.instantiate();
    //     object.addComponent<MeshRenderer>(*e_spheresMesh, irradianceMap, prefilterMap, brdfTexture);
    // }
    //
    // {
    //     // Camera
    //     auto& object = engine.instantiate();
    //     object.transform().setTranslation({0, 20, 0});
    //     object.transform().setRotation(glm::quat(glm::vec3(glm::radians(-90.0f), glm::radians(0.0f), glm::radians(0.0f))));
    //
    //     const auto & camera = object.addComponent<Camera>(WIDTH, HEIGHT, 60);
    //     engine.setCamera(camera);
    //     // object.addComponent<Rotator>();
    //     object.addComponent<CameraController>(glm::vec3(0, 0, 0), 20);
    // }

    auto & map = engine.instantiate();
    map.addComponent<MapController>(irradianceMap, prefilterMap, brdfTexture);

    {
        // Ancient
        auto & object = engine.instantiate();
        auto & animator = object.addComponent<Animator>(ancientMesh);
        auto & meshRenderer = object.addComponent<MeshRenderer>(ancientMesh, irradianceMap, prefilterMap,
                                                                brdfTexture);
        auto & ui = object.addComponent<UserInterface>("Character");
        ui.addBlock<DisplayInterfaceBlock>(1);
        ui.addBlock<AnimationInterfaceBlock>(2);

        // object.addComponent<PlayerController>();
        // object.addComponent<Rotator>(glm::vec3(0.0f, 1.0f, 0.0f));
        meshRenderer.setAnimator(animator);
        animator.setAnimation(0);
    }

    {
        // Camera
        auto & object = engine.instantiate();
        object.transform().setTranslation({0, 3, -3.5});
        object.transform().setRotation(glm::quat(glm::vec3(glm::radians(-15.0f), glm::radians(180.0f), 0)));

        object.addComponent<CameraController>(glm::vec3(0, 1, 0), 2);

        const auto & camera = object.addComponent<Camera>(WIDTH, HEIGHT, 60);
        engine.setCamera(camera);
    }


    // ********************************
    // Run game loop
    // ********************************

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
