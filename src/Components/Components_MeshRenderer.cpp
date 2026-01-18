//
// Created by Simon Cros on 1/29/25.
//

module;

#include "glm/gtc/type_ptr.hpp"
#include "glad/gl.h"

module Components;
import std;
import Engine;
import Engine.RenderInfo;
import OpenGL;

auto MeshRenderer::renderMesh(Engine & engine, const int meshIndex, const glm::mat4 & transform) -> void
{
    const auto & meshRenderInfo = m_mesh.renderInfo().meshes[meshIndex];

    for (int p = 0; p < meshRenderInfo.primitivesCount; ++p)
    {
        const auto & primitiveRenderInfo = meshRenderInfo.primitives[p];

        auto & vertexArray = engine.getVertexArray(primitiveRenderInfo.vertexArrayFlags);
        engine.bindVertexArray(vertexArray);

        glVertexAttrib3f(1, 0, 0, 0); // Normal
        glVertexAttrib4f(2, 1, 1, 1, 1); // Color0
        glVertexAttrib2f(3, 0, 0); // TexCoord0
        glVertexAttrib2f(4, 0, 0); // TexCoord1
        glVertexAttrib4f(5, 0, 0, 0, 0); // Tangent

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

        auto & program = engine.getShaderManager().getProgram(primitiveRenderInfo.programIndex);
        engine.useProgram(program);

        program.setMat4("u_transform", transform);

        engine.bindCubemap(0, m_irradianceMap.id());
        engine.bindCubemap(1, m_prefilterMap.id());
        engine.bindTexture(2, m_brdfLUT.id());
        program.setInt("u_irradianceMap", 0);
        program.setInt("u_prefilterMap", 1);
        program.setInt("u_brdfLUT", 2);

        if (primitiveRenderInfo.material >= 0)
        {
            const auto & material = m_mesh.renderInfo().materials[primitiveRenderInfo.material];

            engine.setDoubleSided(material.doubleSided);
            engine.setBlendEnabled(material.blend);

            if (material.pbr.baseColorTexture.index >= 0)
            {
                engine.bindTexture(3, m_mesh.texture(material.pbr.baseColorTexture.index));
                program.setInt("u_baseColorTexture", 3);
                program.setUint("u_baseColorTexCoordIndex", material.pbr.baseColorTexture.texCoord);
            }

            if (material.pbr.metallicRoughnessTexture.index >= 0)
            {
                engine.bindTexture(4, m_mesh.texture(material.pbr.metallicRoughnessTexture.index));
                program.setInt("u_metallicRoughnessMap", 4);
                program.setUint("u_metallicRoughnessTexCoordIndex",
                                material.pbr.metallicRoughnessTexture.texCoord);
            }

            if (material.normalTexture.index >= 0)
            {
                engine.bindTexture(5, m_mesh.texture(material.normalTexture.index));
                program.setInt("u_normalMap", 5);
                program.setUint("u_normalTexCoordIndex", material.normalTexture.texCoord);
            }

            if (material.emissiveTexture.index >= 0)
            {
                engine.bindTexture(6, m_mesh.texture(material.emissiveTexture.index));
                program.setInt("u_emissiveMap", 6);
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

    const glm::mat4 globalInverseTransform = glm::inverse(transform);

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
