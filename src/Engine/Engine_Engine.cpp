//
// Created by Simon Cros on 1/13/25.
//

module;

#include "42runConfig.h"
#include "glad/gl.h"
#include "glm/glm.hpp"
#include "tiny_gltf.h"
#include "GLFW/glfw3.h"

module Engine;
import std;
import OpenGL;
import Window;

static auto onKeyPressed(const Window & window, const int key, const int action, int mode) -> void
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        window.setShouldClose();
}

auto Engine::Create(Window && window) -> Engine
{
    return Engine(std::move(window));
}

Engine::Engine(Window && window) noexcept : m_window(std::move(window))
{
    m_window.setAsCurrentContext();
    const int version = gladLoadGL(glfwGetProcAddress);
    std::cout << "OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << std::endl;

    const bool hasDebugOutput = GLAD_GL_KHR_debug || GLAD_GL_ARB_debug_output;

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    glEnable(GL_DEPTH_TEST);
    if (hasDebugOutput && (flags & GL_CONTEXT_FLAG_DEBUG_BIT))
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    // TODO TMP only one global VAO for testing
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    getWindow().setKeyCallback(onKeyPressed);
}

auto Engine::run() -> std::expected<void, std::string>
{
    if (m_camera == nullptr)
    {
        return std::unexpected("You must define a camera.");
    }

    if (m_doubleSided)
        glDisable(GL_CULL_FACE);
    else
        glEnable(GL_CULL_FACE);
    if (m_blendEnabled)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
    glDepthMask(m_depthMaskEnabled ? GL_TRUE : GL_FALSE);

    glPolygonMode(GL_FRONT_AND_BACK, m_polygonMode);

    // glClearColor(0.4705882353f, 0.6549019608f, 1.0f, 1.0f);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    m_start = ClockType::now();

    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);

    int fb_w, fb_h;
    glfwGetFramebufferSize(m_window.getGLFWHandle(), &fb_w, &fb_h);
    glViewport(0, 0, fb_w, fb_h);

    // create floating point color buffer
    unsigned int colorBuffer;
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(fb_w),
                 static_cast<GLsizei>(fb_h), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // create depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenTextures(1, &rboDepth);
    glBindTexture(GL_TEXTURE_2D, rboDepth);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT, // format interne
        static_cast<GLsizei>(fb_w),
        static_cast<GLsizei>(fb_h),
        0,
        GL_DEPTH_COMPONENT, // format
        GL_FLOAT, // type (GL_UNSIGNED_BYTE ou GL_FLOAT selon besoin)
        NULL
    );

    // attach buffers
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rboDepth, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto previousTime = m_start;
    bool timeScaleKeyPressed = false;
    while (m_window.update())
    {
        for (SlotSet<Object>::SizeType objectIdx = 0; objectIdx < m_objects.size(); ++objectIdx)
        {
            Object & object = m_objects[objectIdx];
            if (object.isActive())
            {
                object.willUpdate(*this);
            }
        }

        for (SlotSet<Object>::SizeType objectIdx = 0; objectIdx < m_objects.size(); ++objectIdx)
        {
            Object & object = m_objects[objectIdx];
            if (object.isActive())
            {
                object.update(*this);
            }
        }

        const auto pvMat = m_camera->projectionMatrix() * m_camera->computeViewMatrix();
        for (auto & program: m_shaderManager.getPrograms())
        {
            useProgram(program);
            program.setVec3("u_cameraPosition", m_camera->object().transform().translation());
            //program.setVec4("u_fogColor", glm::vec4(0.4705882353f, 0.6549019608f, 1.0f, 1.0f));
            program.setVec3("u_lightPosition", {4, 5, 8});
            program.setMat4("u_projectionView", pvMat);
        }

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (SlotSet<Object>::SizeType objectIdx = 0; objectIdx < m_objects.size(); ++objectIdx)
        {
            Object & object = m_objects[objectIdx];
            if (object.isActive())
            {
                object.render(*this);
            }
        }

        for (SlotSet<Object>::SizeType objectIdx = 0; objectIdx < m_objects.size(); ++objectIdx)
        {
            Object & object = m_objects[objectIdx];
            if (object.isActive())
            {
                object.postRender(*this);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        // TODO cache
        const auto programIdx = *m_shaderManager.getOrCreateShaderProgram(
            *m_shaderManager.getOrAddShaderFile(RESOURCE_PATH"shaders/texcoord.vert"),
            *m_shaderManager.getOrAddShaderFile(RESOURCE_PATH"shaders/hdr.frag"), ShaderFlags::None);
        auto & program = m_shaderManager.getProgram(programIdx);
        useProgram(program);
        bindTexture(0, colorBuffer);
        program.setBool("u_hdr", true);
        program.setFloat("u_exposure", 1.0f);
        renderQuad();
        m_currentBoundVertexArray = 0;
        m_currentBoundArrayBuffer = 0;

        m_window.swapBuffers();

        const auto newTime = ClockType::now();
        ++m_currentFrameInfo.frameCount;

        if (controls().isPressed(GLFW_KEY_UP))
        {
            if (!timeScaleKeyPressed)
            {
                m_currentFrameInfo.timeScale += 0.1f;
            }
            timeScaleKeyPressed = true;
        }
        else if (controls().isPressed(GLFW_KEY_DOWN))
        {
            if (!timeScaleKeyPressed)
            {
                m_currentFrameInfo.timeScale -= 0.1f;
            }
            timeScaleKeyPressed = true;
        }
        else
        {
            timeScaleKeyPressed = false;
        }

        m_currentFrameInfo.timeScale = std::clamp(m_currentFrameInfo.timeScale, 0.0f, 3.0f);

        m_currentFrameInfo.realDeltaTime = newTime - previousTime;
        m_currentFrameInfo.realTime += m_currentFrameInfo.realDeltaTime;

        m_currentFrameInfo.deltaTime = (newTime - previousTime) * m_currentFrameInfo.timeScale;
        m_currentFrameInfo.time += m_currentFrameInfo.deltaTime;

        previousTime = newTime;
    }

    return {};
}

auto Engine::loadModel(const std::string_view & id, const std::string & path,
                       const bool binary) -> std::expected<ModelRef, std::string>
{
    std::string err;
    std::string warn;

    tinygltf::Model rawModel;
    bool loadResult = binary
                          ? m_loader.LoadBinaryFromFile(&rawModel, &err, &warn, path)
                          : m_loader.LoadASCIIFromFile(&rawModel, &err, &warn, path);

    if (!loadResult)
        return std::unexpected(std::move(err));

    if (!warn.empty())
        std::cout << "[WARN] " << warn << std::endl;

    auto model = Model::Create(*this, rawModel);

    const auto & modelRenderInfo = model.renderInfo();
    for (size_t i = 0; i < modelRenderInfo.meshesCount; i++)
    {
        const auto & meshRenderInfo = modelRenderInfo.meshes[i];

        for (size_t j = 0; j < meshRenderInfo.primitivesCount; j++)
        {
            const auto & primitiveRenderInfo = meshRenderInfo.primitives[j];
            if (!m_vertexArrays.contains(primitiveRenderInfo.vertexArrayFlags))
            {
                m_vertexArrays.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(primitiveRenderInfo.vertexArrayFlags),
                                       std::forward_as_tuple(
                                           VertexArray::Create(primitiveRenderInfo.vertexArrayFlags)));
            }
        }
    }

    // C++ 26 will avoid new key allocation if key already exist (remove explicit std::string constructor call).
    // In this function, unnecessary string allocation is not really a problem since we should not try to add two shaders with the same id
    auto [it, inserted] = m_models.try_emplace(std::string(id), std::make_unique<Model>(std::move(model)));

    if (!inserted)
        return std::unexpected("A model with the same id already exist");

    return *it->second;
}

auto Engine::instantiate() -> Object &
{
    return m_objects.emplace(*this);
}
