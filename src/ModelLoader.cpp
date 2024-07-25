#include <iostream>

#include "ModelLoader.hpp"

#include "glad/gl.h"

using namespace tinygltf;

namespace ModelLoader
{
    static bool load(const char *filename, Model *outputModel, bool binary)
    {
        TinyGLTF loader;
        Model model;
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
            std::cout << model << std::endl;

            GLuint vao;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            std::map<int, GLuint> vbos;
            for (auto mesh : model.meshes)
            {
                for (const auto primitive : mesh.primitives)
                {
                    for (const auto attribute : primitive.attributes)
                    {
                        const auto accessor = model.accessors[attribute.second];
                        const auto bufferView = model.bufferViews[accessor.bufferView];

                        if (bufferView.target == 0)
                        {
                            std::cout << "WARN: bufferView.target is zero" << std::endl;
                            continue; // Unsupported bufferView.
                        }

                        if (vbos.count(accessor.bufferView) == 0)
                        {
                            const auto buffer = model.buffers[bufferView.buffer];
                            GLuint glBuffer = 0;

                            glGenBuffers(1, &glBuffer);
                            glBindBuffer(bufferView.target, glBuffer);
                            glBufferStorage(bufferView.target, bufferView.byteLength, &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

                            vbos[accessor.bufferView] = glBuffer;
                        }

                        int byteStride = accessor.ByteStride(bufferView);
                        glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

                        int size = GetNumComponentsInType(accessor.type);
                        if (size < 0 || size > 4)
                        {
                            std::cerr << "WARN: matrices are not supported as per vertex attribute";
                            continue;
                        }

                        int vaa = -1;
                        if (attribute.first.compare("POSITION") == 0)
                            vaa = 0;
                        else if (attribute.first.compare("NORMAL") == 0)
                            vaa = 1;
                        else if (attribute.first.compare("TEXCOORD_0") == 0)
                            vaa = 2;

                        if (vaa > -1)
                        {
                            glEnableVertexAttribArray(vaa);
                            glVertexAttribPointer(vaa, size, accessor.componentType,
                                                  accessor.normalized ? GL_TRUE : GL_FALSE,
                                                  byteStride, (void *)accessor.byteOffset);
                        }
                        else
                        {
                            std::cerr << "WARN: unknown attribute `" << attribute.first << '`' << std::endl;
                        }
                    }

                    if (primitive.indices > 0)
                    {
                        const auto accessor = model.accessors[primitive.indices];
                        if (vbos.count(accessor.bufferView) == 0)
                        {
                            const auto bufferView = model.bufferViews[accessor.bufferView];
                            const auto buffer = model.buffers[bufferView.buffer];
                            GLuint glBuffer = 0;

                            glGenBuffers(1, &glBuffer);
                            glBindBuffer(bufferView.target, glBuffer);
                            glBufferStorage(bufferView.target, bufferView.byteLength, &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

                            vbos[accessor.bufferView] = glBuffer;
                        }
                    }
                }
            }

            *outputModel = model;
        }
        else
        {
            std::cout << "Failed to load glTF: " << filename << std::endl;
        }

        return res;
    }

    bool loadBinary(const char *filename, Model *model)
    {
        return load(filename, model, true);
    }

    bool loadAscii(const char *filename, Model *model)
    {
        return load(filename, model, false);
    }

    std::ostream &operator<<(std::ostream &os, const Model &model)
    {
        os << "buffers : " << model.buffers.size() << '\n';
        os << "bufferviews : " << model.bufferViews.size() << '\n';
        for (auto &mesh : model.meshes)
        {
            os << "mesh : " << mesh.name << '\n';
            for (auto &primitive : mesh.primitives)
            {
                const Accessor &indexAccessor = model.accessors[primitive.indices];

                os << "indexaccessor: count " << indexAccessor.count << ", type "
                   << indexAccessor.componentType << '\n';

                const Material &mat = model.materials[primitive.material];
                for (auto &mats : mat.values)
                {
                    os << "mat : " << mats.first.c_str() << '\n';
                }

                for (auto &image : model.images)
                {
                    os << "image name : " << image.uri << '\n';
                    os << "  size : " << image.image.size() << '\n';
                    os << "  w/h : " << image.width << "/" << image.height << '\n';
                }

                os << "indices : " << primitive.indices << '\n';
                os << "mode : " << "(" << primitive.mode << ")" << '\n';

                for (auto &attrib : primitive.attributes)
                {
                    os << "attribute : " << attrib.first.c_str() << '\n';
                }
            }
        }

        return os;
    }
}
