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

constexpr GLuint cubemapSize = 512;

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
    // Load models
    // ********************************

    stbi_set_flip_vertically_on_load(false);
    std::cout << "Loading models... " << std::endl;
    std::cout << "    character... " << std::flush;
    TRY_V(auto, characterMesh, engine.loadModel("character", RESOURCE_PATH"models/character.glb", true));
    std::cout << "OK!" << std::endl;
    std::cout << "    floor... " << std::flush;
    TRY_V(auto, floorMesh, engine.loadModel("floor", RESOURCE_PATH"models/floor.glb", true));
    std::cout << "OK!" << std::endl;
    std::cout << "    desk... " << std::flush;
    TRY_V(auto, deskMesh, engine.loadModel("desk", RESOURCE_PATH"models/desk.glb", true));
    std::cout << "OK!" << std::endl;
    stbi_set_flip_vertically_on_load(true);


    // ********************************
    // Create shaders
    // ********************************

    TRY_V(const SlotSetIndex, defaultVertShaderIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/pbr.vert"));
    TRY_V(const SlotSetIndex, texcoordVertShaderIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/texcoord.vert"));
    TRY_V(const SlotSetIndex, cubemapVertShaderIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/cubemap.vert"));
    TRY_V(const SlotSetIndex, skyboxVertShaderIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/skybox.vert"));

    TRY_V(const SlotSetIndex, defaultFragShaderIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/pbr.frag"));
    TRY_V(const SlotSetIndex, hdrFragShaderIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/hdr.frag"));
    TRY_V(const SlotSetIndex, brdfFragShaderIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/brdf.frag"));
    TRY_V(const SlotSetIndex, equirectangularFragShaderIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/equirectangular.frag"));
    TRY_V(const SlotSetIndex, irradianceFragShaderIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/irradiance.frag"));
    TRY_V(const SlotSetIndex, prefilterFragShaderIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/prefilter.frag"));
    TRY_V(const SlotSetIndex, skyboxFragShaderIdx, engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/skybox.frag"));


    // ********************************
    // Create IBL resources
    // ********************************

    TRY_V(auto, irradianceMap, OpenGL::Cubemap::builder(stateCache.get())
        .internalFormat(GL_RGB32F)
        .size(cubemapSize)
        .baseLevel(0)
        .maxLevel(4)
        .debugLabel("Irradiance")
        .build());

    TRY_V(auto, prefilterMap, OpenGL::Cubemap::builder(stateCache.get())
        .internalFormat(GL_RGB32F)
        .size(cubemapSize)
        .filtering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)
        .baseLevel(0)
        .maxLevel(4)
        .debugLabel("Prefilter")
        .build());

    TRY_V(auto, brdfTexture, OpenGL::Texture2D::builder(stateCache.get())
        .internalFormat(GL_RG16F)
        .size(cubemapSize, cubemapSize)
        .debugLabel("BRDF")
        .build());


    // ********************************
    // Create programs
    // ********************************

    TRY_V(const SlotSetIndex, hdrProgramIdx, engine.getShaderManager().getOrCreateShaderProgram(texcoordVertShaderIdx, hdrFragShaderIdx, ShaderFlags::None));
    TRY_V(const SlotSetIndex, brdfProgramIdx, engine.getShaderManager().getOrCreateShaderProgram(texcoordVertShaderIdx, brdfFragShaderIdx, ShaderFlags::None));
    TRY_V(const SlotSetIndex, eqProgramIdx, engine.getShaderManager().getOrCreateShaderProgram(cubemapVertShaderIdx, equirectangularFragShaderIdx, ShaderFlags::None));
    TRY_V(const SlotSetIndex, irradianceProgramIdx, engine.getShaderManager().getOrCreateShaderProgram(cubemapVertShaderIdx, irradianceFragShaderIdx, ShaderFlags::None));
    TRY_V(const SlotSetIndex, prefilterProgramIdx, engine.getShaderManager().getOrCreateShaderProgram(cubemapVertShaderIdx, prefilterFragShaderIdx, ShaderFlags::None));
    TRY_V(const SlotSetIndex, skyboxProgramIdx, engine.getShaderManager().getOrCreateShaderProgram(skyboxVertShaderIdx, skyboxFragShaderIdx, ShaderFlags::None));

    TRY(characterMesh.get().prepareShaderPrograms(engine.getShaderManager(), defaultVertShaderIdx, defaultFragShaderIdx));
    TRY(floorMesh.get().prepareShaderPrograms(engine.getShaderManager(), defaultVertShaderIdx, defaultFragShaderIdx));
    TRY(deskMesh.get().prepareShaderPrograms(engine.getShaderManager(), defaultVertShaderIdx, defaultFragShaderIdx));

    // ********************************
    // Compile and link programs
    // ********************************

    std::cout << "Compiling programs... " << std::flush;
    TRY(engine.getShaderManager().reloadAllShaders());
    std::cout << "OK!" << std::endl;


    // ********************************
    // Build IBL
    // ********************************

    std::cout << "Building IBL... " << std::flush;
    const bool irradianceLoaded = irradianceMap.fromCache(".cache/irradiance.cubemap", GL_RGB, GL_FLOAT);
    const bool prefilterLoaded = prefilterMap.fromCache(".cache/prefilter.cubemap", GL_RGB, GL_FLOAT);
    if (!irradianceLoaded || !prefilterLoaded)
    {
        TRY_V(auto, hdrImage, Image::Create(RESOURCE_PATH"textures/skybox/san_giuseppe_bridge_1k.hdr"));
        TRY_V(auto, hdrTexture, OpenGL::Texture2D::builder(stateCache.get())
            .internalFormat(GL_RGB32F)
            .size(hdrImage.width(), hdrImage.height())
            .debugLabel("Equirectangular Skybox")
            .build());
        TRY_V(auto, cubemap, OpenGL::Cubemap::builder(stateCache.get())
            .internalFormat(GL_RGB32F)
            .size(cubemapSize)
            .debugLabel("Skybox")
            .build());

        hdrTexture.fromRaw(hdrImage.glFormat(), hdrImage.glType(), hdrImage.data());
        TRY(cubemap.fromEquirectangular(engine.getShaderManager().getProgram(eqProgramIdx), hdrTexture));

        if (!irradianceLoaded)
        {
            TRY(irradianceMap.fromCubemap(engine.getShaderManager().getProgram(irradianceProgramIdx), cubemap, 0));
            TRY(irradianceMap.saveCache(".cache/irradiance.cubemap", GL_RGB, GL_FLOAT));
        }

        if (!prefilterLoaded)
        {
            TRY(prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), cubemap, 0));
            TRY(prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), cubemap, 1));
            TRY(prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), cubemap, 2));
            TRY(prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), cubemap, 3));
            TRY(prefilterMap.fromCubemap(engine.getShaderManager().getProgram(prefilterProgramIdx), cubemap, 4));
            TRY(prefilterMap.saveCache(".cache/prefilter.cubemap", GL_RGB, GL_FLOAT));
        }
    }

    if (!brdfTexture.fromCache(".cache/brdf.texture2d", GL_RG, GL_HALF_FLOAT))
    {
        TRY(brdfTexture.fromShader(engine.getShaderManager().getProgram(brdfProgramIdx)));
        TRY_LOG(brdfTexture.saveCache(".cache/brdf.texture2d", GL_RG, GL_HALF_FLOAT));
    }
    std::cout << "OK!" << std::endl;


    // ********************************
    // Create the scene
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
        auto & animator = object.addComponent<Animator>(characterMesh);
        auto & meshRenderer = object.addComponent<MeshRenderer>(characterMesh, irradianceMap, prefilterMap,
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
