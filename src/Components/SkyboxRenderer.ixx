//
// Created by Simon Cros on 23/03/2025.
//

module;
#include "42runConfig.h"
#include "glad/gl.h"
#include "glm/glm.hpp"

export module Components:SkyboxRenderer;
import std;
import Engine;
import OpenGL;
import OpenGL.Cubemap;
import Utility.SlotSet;

export class SkyboxRenderer : public Component
{
    static constexpr float skyboxVertices[] = {
        // positions
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };

private:
    VertexArray m_vao;
    GLuint m_vbo;
    OpenGL::Cubemap& m_cubemap;

    void renderSkybox(Engine& engine)
    {
        SlotSetIndex program = *engine.getShaderManager().getOrCreateShaderProgram(
                *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/skybox.vert"),
            *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/skybox.frag"), ShaderFlags::None);

        const auto pvMat = engine.getCamera()->projectionMatrix() * glm::mat4(glm::mat3(engine.getCamera()->computeViewMatrix()));

        glDepthFunc(GL_LEQUAL);
        engine.bindCubemap(0, m_cubemap.id());
        engine.setDepthMaskEnabled(false);
        engine.setDoubleSided(true);

        engine.useProgram(engine.getShaderManager().getProgram(program));
        engine.getShaderManager().getProgram(program).setInt("u_cubemap", 0);
        engine.getShaderManager().getProgram(program).setMat4("u_projectionView", pvMat);
        engine.bindVertexArray(m_vao);
        engine.bindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        engine.setDepthMaskEnabled(true);
        glDepthFunc(GL_LESS);
    }

public:
    SkyboxRenderer(Object& object, Engine& engine, OpenGL::Cubemap& cubemap) :
        Component(object),
        m_cubemap(cubemap)
    {
        auto vao = VertexArray::Create(VertexArrayHasPosition);
        GLuint vertexBuffer;
        glGenBuffers(1, &vertexBuffer);

        engine.bindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

        m_vao = std::move(vao);
        m_vbo = vertexBuffer;
    }

    auto onPostRender(Engine& engine) -> void override
    {
        renderSkybox(engine);
    }
};
