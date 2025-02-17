//
// Created by Simon Cros on 1/13/25.
//

#include "../Components/Camera.h"
#include "Engine.h"
#include "OpenGL/Debug.h"

auto Engine::Create(Window&& window) -> Engine
{
    return Engine(std::move(window));
}

Engine::Engine(Window&& window) noexcept :
    m_window(std::move(window))
{
    m_window.setAsCurrentContext();
    const int version = gladLoadGL(glfwGetProcAddress);
    std::cout << "OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << std::endl;

    const bool hasDebugOutput = GLAD_GL_KHR_debug || GLAD_GL_ARB_debug_output;

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

    getWindow().setKeyCallback([](const Window& window, const int key, const int action, int mode) -> void
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            window.setShouldClose();
    });

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

auto Engine::run() -> void
{
    assert(m_camera != nullptr && "Camera is null");

    if (m_doubleSided)
        glDisable(GL_CULL_FACE);
    else
        glEnable(GL_CULL_FACE);
    if (m_blendEnabled)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);

    glPolygonMode(GL_FRONT_AND_BACK, m_polygonMode);

    glClearColor(0.4705882353f, 0.6549019608f, 1.0f, 1.0f);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    m_start = ClockType::now();

    auto previousTime = m_start;
    while (m_window.update())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto& object : m_objects)
            object->willUpdate(*this);

        for (const auto& object : m_objects)
            object->update(*this);

        const auto pvMat = m_camera->projectionMatrix() * m_camera->computeViewMatrix();
        for (auto& [id, shader] : m_shaders)
        {
            for (auto& [flags, variant] : shader->programs)
            {
                useProgram(*variant.get());
                variant.get()->setVec3("u_cameraPosition", m_camera->object().transform().translation);
                variant.get()->setVec3("u_lightPosition", {4, 5, 8});
                variant.get()->setMat4("u_projectionView", pvMat);
            }
        }

        for (const auto& object : m_objects)
            object->render(*this);

        for (const auto& object : m_objects)
            object->postRender(*this);

        m_window.swapBuffers();

        const auto newTime = ClockType::now();
        ++m_currentFrameInfo.frameCount;
        m_currentFrameInfo.time = newTime - m_start;
        m_currentFrameInfo.deltaTime = newTime - previousTime;
        previousTime = newTime;
    }
}

auto Engine::makeShaderVariants(const std::string_view& id, const std::string& vertPath,
                                const std::string& fragPath) -> Expected<ShaderProgramVariantsRef, std::string>
{
    auto e_shaderVariants = ShaderProgram::Create(vertPath, fragPath);
    if (!e_shaderVariants)
        return Unexpected(std::move(e_shaderVariants).error());

    // C++ 26 will avoid new key allocation if key already exist (remove explicit std::string constructor call).
    // In this function, unnecessary string allocation is not really a problem since we should not try to add two shaders with the same id
    auto [it, inserted] = m_shaders.try_emplace(std::string(id),
                                                std::make_unique<ShaderProgram>(*std::move(e_shaderVariants)));

    if (!inserted)
        return Unexpected("A shader with the same id already exist");
    return *it->second;
}

auto Engine::loadModel(const std::string_view& id, const std::string& path,
                       const bool binary) -> Expected<ModelRef, std::string>
{
    std::string err;
    std::string warn;

    tinygltf::Model rawModel;
    bool loadResult = binary
                          ? m_loader.LoadBinaryFromFile(&rawModel, &err, &warn, path)
                          : m_loader.LoadASCIIFromFile(&rawModel, &err, &warn, path);

    if (!loadResult)
        return Unexpected(std::move(err));

    if (!warn.empty())
        std::cout << "[WARN] " << warn << std::endl;

    auto model = Mesh::Create(*this, std::move(rawModel));

    const auto& modelRenderInfo = model.renderInfo();
    for (size_t i = 0; i < model.model().meshes.size(); i++)
    {
        const auto& mesh = model.model().meshes[i];
        const auto& meshRenderInfo = modelRenderInfo.meshes[i];

        for (size_t j = 0; j < mesh.primitives.size(); j++)
        {
            const auto& primitiveRenderInfo = meshRenderInfo.primitives[j];
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
    auto [it, inserted] = m_models.try_emplace(std::string(id), std::make_unique<Mesh>(std::move(model)));

    if (!inserted)
        return Unexpected("A model with the same id already exist");

    return *it->second;
}

auto Engine::instantiate() -> Object&
{
    return **m_objects.emplace(std::make_unique<Object>()).first;
}
