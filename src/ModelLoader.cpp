#include "ModelLoader.hpp"

#include "stb_image.h"
#include "glad/gl.h"

#define RESOURCE_PATH "./resources/"
#define TEXTURE_PATH (RESOURCE_PATH "textures/")

ModelLoader::ModelLoader(const std::string_view &filename) : _filename(filename)
{
}

static inline bool EndsWith(const std::string_view &fullString, const std::string_view &ending)
{
    if (fullString.length() >= ending.length())
    {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else
    {
        return false;
    }
}

void ModelLoader::LoadAsync()
{
    std::cout << "Make thread" << std::endl;
    std::thread thread_object(&ModelLoader::LoadThread, this);
}

bool ModelLoader::IsCompleted()
{
    std::lock_guard<std::mutex> g(loadingMutex);

    return completed;
}

bool ModelLoader::IsError()
{
    std::lock_guard<std::mutex> g(loadingMutex);

    return error;
}

bool ModelLoader::IsBinaryFile()
{
    return EndsWith(_filename, ".glb");
}

void ModelLoader::LoadThread()
{
    bool res = LoadWorker();

    loadingMutex.lock();
    completed = true;
    error = !res;
    loadingMutex.unlock();
}

bool ModelLoader::LoadWorker()
{
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    bool res;

    if (IsBinaryFile())
    {
        res = loader.LoadBinaryFromFile(&model, &err, &warn, _filename.c_str());
    }
    else
    {
        res = loader.LoadASCIIFromFile(&model, &err, &warn, _filename.c_str());
    }

    if (!warn.empty())
    {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty())
    {
        std::cout << "ERR: " << err << std::endl;
    }

    if (!res)
    {
        std::cout << "Failed to load glTF: " << _filename << std::endl;
        return res;
    }

    std::cout << "Loaded glTF: " << _filename << std::endl;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

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

            if (primitive.material >= 0)
            {
                const auto &material = model.materials[primitive.material];
                int textureId = material.pbrMetallicRoughness.baseColorTexture.index;
                if (textureId >= 0 && textures.count(textureId) == 0)
                {
                    const auto &texture = model.textures[textureId];

                    assert(texture.source >= 0);

                    const auto &image = model.images[texture.source];

                    if (!image.image.empty())
                    {
                        GLuint glTexture;
                        glGenTextures(1, &glTexture);
                        glBindTexture(GL_TEXTURE_2D, glTexture);

                        if (texture.sampler >= 0)
                        {
                            const auto &sampler = model.samplers[texture.sampler];
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
                            if (sampler.minFilter > -1)
                            {
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
                            }
                            if (sampler.magFilter > -1)
                            {
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
                            }
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
                        else if (image.bits == 32)
                        {
                            type = GL_UNSIGNED_INT;
                        }

                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, format,
                                     type, image.image.data());
                        glGenerateMipmap(GL_TEXTURE_2D);

                        textures[textureId] = glTexture;
                    }
                    else if (!image.uri.empty())
                    {
                        GLuint glTexture;
                        glGenTextures(1, &glTexture);
                        glBindTexture(GL_TEXTURE_2D, glTexture);

                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                        int width, height, nrChannels;
                        stbi_uc *data = stbi_load((TEXTURE_PATH + image.uri).c_str(), &width, &height, &nrChannels, 0);
                        if (data != nullptr)
                        {
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

                            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                            glGenerateMipmap(GL_TEXTURE_2D);
                        }
                        else
                        {
                            std::cerr << "ERROR::TEXTURE::LOADING_FAILED\n"
                                      << stbi_failure_reason() << std::endl;
                        }

                        stbi_image_free(data);
                        textures[textureId] = glTexture;
                    }
                }
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

    glBindVertexArray(0);

    return res;
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
