#include <iostream>

#include "ModelLoader.hpp"

#include "glad/gl.h"

namespace ModelLoader
{
    // static void SetupMeshState(tinygltf::Model &model, GLuint progId)
    // {
    //     // Buffer
    //     {
    //         for (size_t i = 0; i < model.bufferViews.size(); i++)
    //         {
    //             const tinygltf::BufferView &bufferView = model.bufferViews[i];
    //             if (bufferView.target == 0)
    //             {
    //                 std::cout << "WARN: bufferView.target is zero" << std::endl;
    //                 continue; // Unsupported bufferView.
    //             }

    //             int sparse_accessor = -1;
    //             for (size_t a_i = 0; a_i < model.accessors.size(); ++a_i)
    //             {
    //                 const auto &accessor = model.accessors[a_i];
    //                 if (accessor.bufferView == i)
    //                 {
    //                     std::cout << i << " is used by accessor " << a_i << std::endl;
    //                     if (accessor.sparse.isSparse)
    //                     {
    //                         std::cout
    //                             << "WARN: this bufferView has at least one sparse accessor to "
    //                                "it. We are going to load the data as patched by this "
    //                                "sparse accessor, not the original data"
    //                             << std::endl;
    //                         sparse_accessor = a_i;
    //                         break;
    //                     }
    //                 }
    //             }

    //             const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
    //             GLBufferState state;
    //             glGenBuffers(1, &state.vb);
    //             glBindBuffer(bufferView.target, state.vb);
    //             std::cout << "buffer.size= " << buffer.data.size()
    //                       << ", byteOffset = " << bufferView.byteOffset << std::endl;

    //             if (sparse_accessor < 0)
    //                 glBufferData(bufferView.target, bufferView.byteLength,
    //                              &buffer.data.at(0) + bufferView.byteOffset,
    //                              GL_STATIC_DRAW);
    //             else
    //             {
    //                 const auto accessor = model.accessors[sparse_accessor];
    //                 // copy the buffer to a temporary one for sparse patching
    //                 unsigned char *tmp_buffer = new unsigned char[bufferView.byteLength];
    //                 memcpy(tmp_buffer, buffer.data.data() + bufferView.byteOffset,
    //                        bufferView.byteLength);

    //                 const size_t size_of_object_in_buffer =
    //                     ComponentTypeByteSize(accessor.componentType);
    //                 const size_t size_of_sparse_indices =
    //                     ComponentTypeByteSize(accessor.sparse.indices.componentType);

    //                 const auto &indices_buffer_view =
    //                     model.bufferViews[accessor.sparse.indices.bufferView];
    //                 const auto &indices_buffer = model.buffers[indices_buffer_view.buffer];

    //                 const auto &values_buffer_view =
    //                     model.bufferViews[accessor.sparse.values.bufferView];
    //                 const auto &values_buffer = model.buffers[values_buffer_view.buffer];

    //                 for (size_t sparse_index = 0; sparse_index < accessor.sparse.count;
    //                      ++sparse_index)
    //                 {
    //                     int index = 0;
    //                     // std::cout << "accessor.sparse.indices.componentType = " <<
    //                     // accessor.sparse.indices.componentType << std::endl;
    //                     switch (accessor.sparse.indices.componentType)
    //                     {
    //                     case TINYGLTF_COMPONENT_TYPE_BYTE:
    //                     case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    //                         index = (int)*(
    //                             unsigned char *)(indices_buffer.data.data() +
    //                                              indices_buffer_view.byteOffset +
    //                                              accessor.sparse.indices.byteOffset +
    //                                              (sparse_index * size_of_sparse_indices));
    //                         break;
    //                     case TINYGLTF_COMPONENT_TYPE_SHORT:
    //                     case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    //                         index = (int)*(
    //                             unsigned short *)(indices_buffer.data.data() +
    //                                               indices_buffer_view.byteOffset +
    //                                               accessor.sparse.indices.byteOffset +
    //                                               (sparse_index * size_of_sparse_indices));
    //                         break;
    //                     case TINYGLTF_COMPONENT_TYPE_INT:
    //                     case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    //                         index = (int)*(
    //                             unsigned int *)(indices_buffer.data.data() +
    //                                             indices_buffer_view.byteOffset +
    //                                             accessor.sparse.indices.byteOffset +
    //                                             (sparse_index * size_of_sparse_indices));
    //                         break;
    //                     }
    //                     std::cout << "updating sparse data at index  : " << index
    //                               << std::endl;
    //                     // index is now the target of the sparse index to patch in
    //                     const unsigned char *read_from =
    //                         values_buffer.data.data() +
    //                         (values_buffer_view.byteOffset +
    //                          accessor.sparse.values.byteOffset) +
    //                         (sparse_index * (size_of_object_in_buffer * accessor.type));

    //                     /*
    //                     std::cout << ((float*)read_from)[0] << "\n";
    //                     std::cout << ((float*)read_from)[1] << "\n";
    //                     std::cout << ((float*)read_from)[2] << "\n";
    //                     */

    //                     unsigned char *write_to =
    //                         tmp_buffer + index * (size_of_object_in_buffer * accessor.type);

    //                     memcpy(write_to, read_from, size_of_object_in_buffer * accessor.type);
    //                 }

    //                 // debug:
    //                 /*for(size_t p = 0; p < bufferView.byteLength/sizeof(float); p++)
    //                 {
    //                   float* b = (float*)tmp_buffer;
    //                   std::cout << "modified_buffer [" << p << "] = " << b[p] << '\n';
    //                 }*/

    //                 glBufferData(bufferView.target, bufferView.byteLength, tmp_buffer,
    //                              GL_STATIC_DRAW);
    //                 delete[] tmp_buffer;
    //             }
    //             glBindBuffer(bufferView.target, 0);

    //             gBufferState[i] = state;
    //         }
    //     }
    //     glUseProgram(progId);
    //     GLint vtloc = glGetAttribLocation(progId, "in_vertex");
    //     GLint nrmloc = glGetAttribLocation(progId, "in_normal");
    //     GLint uvloc = glGetAttribLocation(progId, "in_texcoord");

    //     // GLint diffuseTexLoc = glGetUniformLocation(progId, "diffuseTex");
    //     GLint isCurvesLoc = glGetUniformLocation(progId, "uIsCurves");

    //     gGLProgramState.attribs["POSITION"] = vtloc;
    //     gGLProgramState.attribs["NORMAL"] = nrmloc;
    //     gGLProgramState.attribs["TEXCOORD_0"] = uvloc;
    //     // gGLProgramState.uniforms["diffuseTex"] = diffuseTexLoc;
    //     gGLProgramState.uniforms["isCurvesLoc"] = isCurvesLoc;
    // };

    static bool load(const char *filename, tinygltf::Model *outputModel, GLuint *vao, std::map<int, GLuint> &buffers, std::map<int, GLuint> &textures,
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

            for (auto &mesh : model.meshes)
            {
                for (const auto &primitive : mesh.primitives)
                {
                    if (primitive.indices < 0)
                        continue;

                    for (const auto &[attributeName, accessorId] : primitive.attributes)
                    {
                        const auto &accessor = model.accessors[accessorId];
                        const auto &bufferView = model.bufferViews[accessor.bufferView];

                        if (bufferView.target == 0)
                        {
                            std::cout << "WARN: bufferView.target is zero" << std::endl;
                            continue; // Unsupported bufferView.
                        }

                        if (buffers.count(accessor.bufferView) == 0)
                        {
                            const auto &buffer = model.buffers[bufferView.buffer];
                            GLuint glBuffer = 0;

                            glGenBuffers(1, &glBuffer);
                            glBindBuffer(bufferView.target, glBuffer);
                            glBufferData(bufferView.target, static_cast<GLsizeiptr>(bufferView.byteLength),
                                         &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

                            buffers[accessor.bufferView] = glBuffer;
                        }
                    }

                    if (primitive.material > -1)
                    {
                    }

                    {
                        const auto &accessor = model.accessors[primitive.indices];
                        if (buffers.count(accessor.bufferView) == 0)
                        {
                            const auto &bufferView = model.bufferViews[accessor.bufferView];
                            const auto &buffer = model.buffers[bufferView.buffer];
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

    bool loadBinary(const char *filename, tinygltf::Model *model, GLuint *vao, std::map<int, GLuint> &buffers, std::map<int, GLuint> &textures)
    {
        return load(filename, model, vao, buffers, textures, true);
    }

    bool loadAscii(const char *filename, tinygltf::Model *model, GLuint *vao, std::map<int, GLuint> &buffers, std::map<int, GLuint> &textures)
    {
        return load(filename, model, vao, buffers, textures, false);
    }

    std::ostream &operator<<(std::ostream &os, const tinygltf::Model &model)
    {
        os << "buffers : " << model.buffers.size() << '\n';
        os << "bufferviews : " << model.bufferViews.size() << '\n';
        for (auto &mesh : model.meshes)
        {
            os << "mesh : " << mesh.name << '\n';
            for (auto &primitive : mesh.primitives)
            {
                const auto &indexAccessor = model.accessors[primitive.indices];

                os << "indexaccessor: count " << indexAccessor.count << ", type "
                   << indexAccessor.componentType << '\n';

                const auto &mat = model.materials[primitive.material];
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
