#include <iostream>

#include "ModelLoader.hpp"

#include "glad/gl.h"

namespace ModelLoader
{
    static bool load(const char* filename, tinygltf::Model* outputModel, GLuint* vao, std::map<int, GLuint>& buffers,
                     std::map<int, GLuint>& textures,
                     bool binary)
    {
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string err;
        std::string warn;
        bool res;

        if (binary)
        {
            res = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
        }
        else
        {
            res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
        }

        if (!warn.empty())
        {
            std::cout << "WARN: " << warn << std::endl;
        }

        if (!err.empty())
        {
            std::cout << "ERR: " << err << std::endl;
        }

        if (res)
        {
            std::cout << "Loaded glTF: " << filename << std::endl;

            glGenVertexArrays(1, vao);
            glBindVertexArray(*vao);

            for (auto& mesh : model.meshes)
            {
                for (const auto& primitive : mesh.primitives)
                {
                    if (primitive.indices < 0)
                        continue;

                    for (const auto& [attributeName, accessorId] : primitive.attributes)
                    {
                        const auto& accessor = model.accessors[accessorId];
                        const auto& bufferView = model.bufferViews[accessor.bufferView];

                        if (bufferView.target == 0)
                        {
                            std::cout << "WARN: bufferView.target is zero" << std::endl;
                            continue; // Unsupported bufferView.
                        }

                        if (buffers.count(accessor.bufferView) == 0)
                        {
                            const auto& buffer = model.buffers[bufferView.buffer];
                            GLuint glBuffer = 0;

                            glGenBuffers(1, &glBuffer);
                            glBindBuffer(bufferView.target, glBuffer);
                            glBufferData(bufferView.target, static_cast<GLsizeiptr>(bufferView.byteLength),
                                         &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

                            buffers[accessor.bufferView] = glBuffer;
                        }
                    }

                    if (primitive.material >= 0)
                    {
                        const auto& material = model.materials[primitive.material];
                        int textureId = material.pbrMetallicRoughness.baseColorTexture.index;
                        if (textureId >= 0)
                        {
                            const auto& texture = model.textures[textureId];

                            assert(texture.source >= 0);

                            const auto& image = model.images[texture.source];

                            if (!image.image.empty() && image.as_is)
                            {
                                GLuint glTexture;
                                glGenTextures(1, &glTexture);
                                glBindTexture(GL_TEXTURE_2D, glTexture);

                                if (texture.sampler >= 0)
                                {
                                    const auto& sampler = model.samplers[texture.sampler];
                                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
                                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
                                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
                                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
                                }
                                else
                                {
                                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                                }

                                GLenum format = GL_RGBA;
                                if (image.component == 1)
                                {
                                    format = GL_RED;
                                }
                                else if (image.component == 2)
                                {
                                    format = GL_RG;
                                }
                                else if (image.component == 3)
                                {
                                    format = GL_RGB;
                                }

                                GLenum type = GL_UNSIGNED_BYTE;
                                if (image.bits == 16)
                                {
                                    type = GL_UNSIGNED_SHORT;
                                }

                                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, format,
                                             type, image.image.data());
                                glGenerateMipmap(GL_TEXTURE_2D);
                                textures[textureId] = glTexture;
                            }
                            else if (image.as_is)
                            {
                                std::cout << "TODO Image as_is possible (texture: " << texture.name << ")" << std::endl;
                            }
                            else
                            {
                                // TODO
                            }
                        }
                    }

                    {
                        const auto& accessor = model.accessors[primitive.indices];
                        if (buffers.count(accessor.bufferView) == 0)
                        {
                            const auto& bufferView = model.bufferViews[accessor.bufferView];
                            const auto& buffer = model.buffers[bufferView.buffer];
                            GLuint glBuffer = 0;

                            glGenBuffers(1, &glBuffer);
                            glBindBuffer(bufferView.target, glBuffer);
                            glBufferData(bufferView.target, static_cast<GLsizeiptr>(bufferView.byteLength),
                                         &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

                            buffers[accessor.bufferView] = glBuffer;
                        }
                    }
                }
            }

            *outputModel = model;
            glBindVertexArray(0);
        }
        else
        {
            std::cout << "Failed to load glTF: " << filename << std::endl;
        }

        return res;
    }

    bool loadBinary(const char* filename, tinygltf::Model* model, GLuint* vao, std::map<int, GLuint>& buffers,
                    std::map<int, GLuint>& textures)
    {
        return load(filename, model, vao, buffers, textures, true);
    }

    bool loadAscii(const char* filename, tinygltf::Model* model, GLuint* vao, std::map<int, GLuint>& buffers,
                   std::map<int, GLuint>& textures)
    {
        return load(filename, model, vao, buffers, textures, false);
    }

    std::ostream& operator<<(std::ostream& os, const tinygltf::Model& model)
    {
        os << "buffers : " << model.buffers.size() << '\n';
        os << "bufferviews : " << model.bufferViews.size() << '\n';
        for (auto& mesh : model.meshes)
        {
            os << "mesh : " << mesh.name << '\n';
            for (auto& primitive : mesh.primitives)
            {
                const auto& indexAccessor = model.accessors[primitive.indices];

                os << "indexaccessor: count " << indexAccessor.count << ", type "
                    << indexAccessor.componentType << '\n';

                const auto& mat = model.materials[primitive.material];
                for (auto& mats : mat.values)
                {
                    os << "mat : " << mats.first.c_str() << '\n';
                }

                for (auto& image : model.images)
                {
                    os << "image name : " << image.uri << '\n';
                    os << "  size : " << image.image.size() << '\n';
                    os << "  w/h : " << image.width << "/" << image.height << '\n';
                }

                os << "indices : " << primitive.indices << '\n';
                os << "mode : " << "(" << primitive.mode << ")" << '\n';

                for (auto& attrib : primitive.attributes)
                {
                    os << "attribute : " << attrib.first.c_str() << '\n';
                }
            }
        }

        return os;
    }
}
