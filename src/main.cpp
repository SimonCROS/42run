//
// Created by Simon Cros on 3/1/25.
//

#include "glad/gl.h"
#include "glm/glm.hpp"
#include <cstdlib>

#include "42runConfig.h"
#include "stb_image.h"
#include "tiny_gltf.h"

import std;
import Components;
import Engine;
import InterfaceBlocks;
import Window;
import Image;
import OpenGL;
import OpenGL.StateCache;
import OpenGL.Texture2D2;
import OpenGL.Cubemap2;
import Utility.SlotSet;

class Rotator : public Component
{
private:
    glm::vec3 m_axis;

public:
    explicit Rotator(Object& object, const glm::vec3 axis) : Component(object), m_axis(axis) {  }

    auto onUpdate(Engine& engine) -> void override
    {
        object().transform().rotate(glm::quat(m_axis * glm::radians(0.2f)));
    }
};

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

    // TODO safe
    const SlotSetIndex defaultVertIdx = *engine.getShaderManager().getOrAddShaderFile(
        RESOURCE_PATH"shaders/pbr.vert");
    const SlotSetIndex defaultFragIdx = *engine.getShaderManager().getOrAddShaderFile(
        RESOURCE_PATH"shaders/pbr.frag");

    auto e_spheresMesh = engine.loadModel("spheres", RESOURCE_PATH"models/spheres_smooth.glb", true);
    if (!e_spheresMesh)
        return std::unexpected("Failed to load model: " + std::move(e_spheresMesh).error());

    *engine.getShaderManager().getOrCreateShaderProgram(
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/texcoord.vert"),
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/hdr.frag"), ShaderFlags::None);

    const auto brdfProgramIdx = *engine.getShaderManager().getOrCreateShaderProgram(
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/texcoord.vert"),
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/brdf.frag"), ShaderFlags::None);

    const auto eqProgramIdx = *engine.getShaderManager().getOrCreateShaderProgram(
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/cubemap.vert"),
    *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/equirectangular_to_cubemap.frag"), ShaderFlags::None);

    const auto irradianceProgramIdx = *engine.getShaderManager().getOrCreateShaderProgram(
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/cubemap.vert"),
    *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/irradiance.frag"), ShaderFlags::None);

    const auto prefilterProgramIdx = *engine.getShaderManager().getOrCreateShaderProgram(
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/cubemap.vert"),
    *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/prefilter.frag"), ShaderFlags::None);

    *engine.getShaderManager().getOrCreateShaderProgram(
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/skybox.vert"),
    *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/skybox.frag"), ShaderFlags::None);

    if (const auto && e_result = e_spheresMesh->get().prepareShaderPrograms(
        engine.getShaderManager(), defaultVertIdx, defaultFragIdx); !e_result)
    {
        return e_result;
    }

    if (const auto && e_result = engine.getShaderManager().reloadAllShaders(); !e_result)
    {
        return e_result;
    }

    auto e_hdrImage = Image::Create(RESOURCE_PATH"textures/skybox/san_giuseppe_bridge_1k.hdr");
    if (!e_hdrImage)
    {
        return std::unexpected(std::move(e_hdrImage).error());
    }

    const GLuint cubemapSize = 512;

    auto hdrTexture = *OpenGL::Texture2D2::builder(stateCache.get()).fromImage(*e_hdrImage, GL_RGB32F).build();
    std::println("{}", *e_hdrImage);
    std::println("{}", hdrTexture);
    auto cubemap2Texture = *OpenGL::Cubemap2::builder(stateCache.get()).withFormat(hdrTexture.internalFormat(), hdrTexture.format(), hdrTexture.type()).withSize(cubemapSize).build();
    *cubemap2Texture.fromEquirectangular(engine.getShaderManager().getProgram(eqProgramIdx), hdrTexture);

    auto irradianceMap = *OpenGL::Cubemap2::builder(stateCache.get()).withFormat(hdrTexture.internalFormat(), hdrTexture.format(), hdrTexture.type()).withSize(cubemapSize).build();
    *irradianceMap.fromCubemap(engine.getShaderManager().getProgram(irradianceProgramIdx), cubemap2Texture, 0);

    auto prefilterMap = *OpenGL::Cubemap2::builder(stateCache.get()).withFormat(hdrTexture.internalFormat(), hdrTexture.format(), hdrTexture.type()).withSize(cubemapSize).withFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR).build();
    *prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), cubemap2Texture, 0);
    *prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), cubemap2Texture, 1);
    *prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), cubemap2Texture, 2);
    *prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), cubemap2Texture, 3);
    *prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), cubemap2Texture, 4);

    // auto brdfTexture = *OpenGL::Texture2D2::builder(stateCache.get()).withDimensions(cubemapSize, cubemapSize).build();
    // brdfTexture.fromShader(brdfTexture);

    {
        auto& object = engine.instantiate();
        object.addComponent<SkyboxRenderer>(engine, irradianceMap);
    }

    {
        auto & object = engine.instantiate();
        object.addComponent<MeshRenderer>(*e_spheresMesh, irradianceMap, prefilterMap);
    }

    {
        // Camera
        auto& object = engine.instantiate();
        object.transform().setTranslation({0, 20, 0});
        object.transform().setRotation(glm::quat(glm::vec3(glm::radians(-90.0f), glm::radians(0.0f), glm::radians(0.0f))));

        const auto & camera = object.addComponent<Camera>(WIDTH, HEIGHT, 60);
        engine.setCamera(camera);
        // object.addComponent<Rotator>();
        object.addComponent<CameraController>(glm::vec3(0, 0, 0), 20);
    }

    // auto e_floorMesh = engine.loadModel("floor", RESOURCE_PATH"models/floor.glb", true);
    // if (!e_floorMesh)
    //     return std::unexpected("Failed to load model: " + std::move(e_floorMesh).error());
    //
    // // TODO safe
    // (void)e_floorMesh->get().prepareShaderPrograms(engine.getShaderManager(), defaultVertIdx, defaultFragIdx);
    //
    // auto e_deskMesh = engine.loadModel("desk", RESOURCE_PATH"models/desk.glb", true);
    // if (!e_deskMesh)
    //     return std::unexpected("Failed to load model: " + std::move(e_deskMesh).error());
    //
    // // TODO safe
    // (void)e_deskMesh->get().prepareShaderPrograms(engine.getShaderManager(), defaultVertIdx, defaultFragIdx);
    //
    // auto e_ancientMesh = engine.loadModel("ancient", RESOURCE_PATH"models/ancient.glb", true);
    // if (!e_ancientMesh)
    //     return std::unexpected("Failed to load model: " + std::move(e_ancientMesh).error());
    //
    // // TODO safe
    // (void)e_ancientMesh->get().prepareShaderPrograms(engine.getShaderManager(), defaultVertIdx, defaultFragIdx);

    // // TODO safe
    // (void)engine.getShaderManager().reloadAllShaders();

    // auto& map = engine.instantiate();
    // map.addComponent<MapController>(cubemapTexture);

    // {
    //     // Ancient
    //     auto& object = engine.instantiate();
    //     object.transform().scale(0.65f);
    //     auto& animator = object.addComponent<Animator>(*e_ancientMesh);
    //     auto& meshRenderer = object.addComponent<MeshRenderer>(*e_ancientMesh, irradianceMap, cubemap2Texture);
    //     object.addComponent<PlayerController>();
    //     meshRenderer.setAnimator(animator);
    //     animator.setAnimation(0);
    // }
    //
    // {
    //     // Camera
    //     auto& object = engine.instantiate();
    //     object.transform().setTranslation({0, 3, -3.5});
    //     object.transform().setRotation(glm::quat(glm::vec3(glm::radians(-15.0f), glm::radians(180.0f), 0)));
    //
    //     const auto& camera = object.addComponent<Camera>(WIDTH, HEIGHT, 60);
    //     engine.setCamera(camera);
    // }

    return engine.run();
}

auto main() -> int
{
    stbi_set_flip_vertically_on_load(true);

    auto e_result = start();
    if (!e_result)
    {
        std::cerr << "Error: " << e_result.error() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
