//
// Created by Simon Cros on 1/29/25.
//

module;

#include "42runConfig.h"
#include "glm/gtc/type_ptr.hpp"
#include "glad/gl.h"

module Components;
import std;
import Engine;
import Engine.RenderInfo;
import OpenGL;

static void * bufferOffset(const size_t offset)
{
    return reinterpret_cast<void *>(offset);
}

auto MeshRenderer::renderMesh(Engine & engine, const int meshIndex, const glm::mat4 & transform) -> void
{
    const auto & meshRenderInfo = m_mesh.renderInfo().meshes[meshIndex];

    for (int p = 0; p < meshRenderInfo.primitivesCount; ++p)
    {
        const auto & primitiveRenderInfo = meshRenderInfo.primitives[p];

        auto & vertexArray = engine.getVertexArray(primitiveRenderInfo.vertexArrayFlags);
        engine.bindVertexArray(vertexArray);

        for (const auto & attribute: primitiveRenderInfo.attributes)
        {
            const auto & accessorRenderInfo = m_mesh.renderInfo().accessors[attribute.accessor];

            const int attributeLocation = static_cast<int>(attribute.type);
            if (attributeLocation != -1)
            {
                engine.bindBuffer(
                    GL_ARRAY_BUFFER,
                    *m_mesh.renderInfo().bufferViews[accessorRenderInfo.bufferView].glBuffer);

                glVertexAttribPointer(attributeLocation,
                                      accessorRenderInfo.componentCount,
                                      accessorRenderInfo.componentType,
                                      accessorRenderInfo.normalized,
                                      accessorRenderInfo.byteStride,
                                      bufferOffset(accessorRenderInfo.byteOffset));
            }
        }

        // TODO have a default material instead of getting a default shader here when no material
        const auto programIdx = primitiveRenderInfo.material >= 0 ? m_mesh.renderInfo().materials[primitiveRenderInfo.material].programIndex :
            *engine.getShaderManager().getOrCreateShaderProgram(*engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/default.vert"),
        *engine.getShaderManager().getOrAddShaderFile(RESOURCE_PATH"shaders/default.frag"), ShaderFlags::None);
        auto & program = engine.getShaderManager().getProgram(programIdx);
        engine.useProgram(program);

        program.setMat4("u_transform", transform);

        engine.bindCubemap(0, m_reflectionCubemap.textureId());
        program.setInt("m_cubemap", 0);

        if (primitiveRenderInfo.material >= 0)
        {
            const auto & material = m_mesh.renderInfo().materials[primitiveRenderInfo.material];

            engine.setDoubleSided(material.doubleSided);
            engine.setBlendEnabled(material.blend);

            if (material.pbr.baseColorTexture.index >= 0)
            {
                engine.bindTexture(1, m_mesh.texture(material.pbr.baseColorTexture.index));
                program.setInt("u_baseColorTexture", 1);
                program.setUint("u_baseColorTexCoordIndex", material.pbr.baseColorTexture.texCoord);
            }

            if (material.pbr.metallicRoughnessTexture.index >= 0)
            {
                engine.bindTexture(2, m_mesh.texture(material.pbr.metallicRoughnessTexture.index));
                program.setInt("u_metallicRoughnessMap", 2);
                program.setUint("u_metallicRoughnessTexCoordIndex",
                                material.pbr.metallicRoughnessTexture.texCoord);
            }

            if (material.normalTexture.index >= 0)
            {
                engine.bindTexture(3, m_mesh.texture(material.normalTexture.index));
                program.setInt("u_normalMap", 3);
                program.setUint("u_normalTexCoordIndex", material.normalTexture.texCoord);
            }

            if (material.emissiveTexture.index >= 0)
            {
                engine.bindTexture(4, m_mesh.texture(material.emissiveTexture.index));
                program.setInt("u_emissiveMap", 4);
                program.setUint("u_emissiveTexCoordIndex", material.emissiveTexture.texCoord);
            }

            program.setVec4("u_baseColorFactor", material.pbr.baseColorFactor);
            program.setFloat("u_metallicFactor", material.pbr.metallicFactor);
            program.setFloat("u_roughnessFactor", material.pbr.roughnessFactor);
            program.setFloat("u_normalScale", material.normalTexture.scale);
            program.setVec3("u_emissiveFactor", material.emissiveFactor);
        }
        else
        {
            engine.setDoubleSided(false);
            engine.setBlendEnabled(false);
            program.setVec4("u_baseColorFactor", glm::vec4(1));
            program.setFloat("u_metallicFactor", 1);
            program.setFloat("u_roughnessFactor", 1);
            program.setFloat("u_normalScale", 1);
            program.setVec3("u_emissiveFactor", glm::vec3(0));
        }

        assert(primitiveRenderInfo.indices >= 0); // TODO handle non indexed primitives

        const auto & accessorRenderInfo = m_mesh.renderInfo().accessors[primitiveRenderInfo.indices];

        engine.bindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            *m_mesh.renderInfo().bufferViews[accessorRenderInfo.bufferView].glBuffer);

        glDrawElements(primitiveRenderInfo.mode,
                       static_cast<GLsizei>(accessorRenderInfo.count),
                       accessorRenderInfo.componentType,
                       bufferOffset(accessorRenderInfo.byteOffset));
    }
}

auto MeshRenderer::renderNodeRecursive(Engine & engine, const int nodeIndex) -> void
{
    const NodeRenderInfo & node = m_mesh.renderInfo().nodes[nodeIndex];

    if (node.mesh > -1)
        renderMesh(engine, node.mesh, m_nodes[nodeIndex].globalTransform);
    for (int i = 0; i < node.childrenCount; ++i)
        renderNodeRecursive(engine, node.children[i]);
}

auto MeshRenderer::calculateGlobalTransformsRecursive(const int nodeIndex, glm::mat4 transform) -> void
{
    const NodeRenderInfo & node = m_mesh.renderInfo().nodes[nodeIndex];

    if (const auto * mat = std::get_if<glm::mat4>(&node.transform))
    {
        transform *= *mat;
    }
    else
    {
        const auto & trs = std::get<TRS>(node.transform);
        const auto & animTRS = m_animator.has_value()
                                   ? m_animator->get().nodeTransform(nodeIndex)
                                   : Animator::AnimatedTransform{};

        transform = glm::translate(transform, animTRS.translation.has_value() ? *animTRS.translation : trs.translation);
        transform *= glm::mat4_cast(animTRS.rotation.has_value() ? *animTRS.rotation : trs.rotation);
        transform = glm::scale(transform, animTRS.scale.has_value() ? *animTRS.scale : trs.scale);
    }

    m_nodes[nodeIndex].globalTransform = transform;

    for (int i = 0; i < node.childrenCount; ++i)
        calculateGlobalTransformsRecursive(node.children[i], transform);
}

auto MeshRenderer::calculateJointMatrices(const int skinIndex, const glm::mat4 & transform) -> void
{
    const auto & skin = m_mesh.renderInfo().skins[skinIndex];
    auto & jointMatrices = m_skins[skinIndex].jointMatrices;

    glm::mat4 globalInverseTransform;
    if (skin.skeleton > -1)
        globalInverseTransform = glm::inverse(m_nodes[skin.skeleton].globalTransform);
    else
        globalInverseTransform = transform;

    for (int i = 0; i < skin.joints.size(); ++i)
    {
        jointMatrices[i] = globalInverseTransform
                           * m_nodes[skin.joints[i]].globalTransform
                           * skin.inverseBindMatrices[i];
    }
}

void MeshRenderer::onRender(Engine & engine)
{
    if (!displayed())
        return;

    if (engine.polygonMode() != m_polygonMode)
        engine.setPolygoneMode(m_polygonMode);

    const auto globalTransform = object().worldTransform();

    const auto & renderInfo = m_mesh.renderInfo();

    for (int i = 0; i < renderInfo.rootNodesCount; ++i)
    {
        calculateGlobalTransformsRecursive(renderInfo.rootNodes[i], globalTransform);
    }

    for (int skinIndex = 0; skinIndex < renderInfo.skinsCount; ++skinIndex)
    {
        calculateJointMatrices(skinIndex, globalTransform);

        const auto jointsUBO = m_mesh.renderInfo().skins[skinIndex].glBuffer;
        const GLuint uniformBlockBinding = skinIndex;
        glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockBinding, jointsUBO);

        for (auto & program: engine.getShaderManager().getPrograms())
        {
            if ((engine.getShaderManager().getShader(program.vertexShaderIdx()).flags() & ShaderFlags::HasSkin) == ShaderFlags::HasSkin)
            {
                program.setUniformBlock("JointMatrices", uniformBlockBinding);
            }
        }

        glBindBuffer(GL_UNIFORM_BUFFER, jointsUBO);
        glBufferSubData(
            GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(m_skins[skinIndex].jointMatrices.size() * sizeof(glm::mat4)),
            m_skins[skinIndex].jointMatrices.data());
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    for (int i = 0; i < renderInfo.rootNodesCount; ++i)
    {
        renderNodeRecursive(engine, renderInfo.rootNodes[i]);
    }
}
