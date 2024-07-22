#include <iostream>

#include "ModelLoader.hpp"

#include "glad/gl.h"

using namespace tinygltf;

namespace ModelLoader
{
    static bool load(const char *filename, bool binary)
    {
        TinyGLTF loader;
        Model tinyModel;
        std::string err;
        std::string warn;
        bool res;

        if (binary)
        {
            res = loader.LoadBinaryFromFile(&tinyModel, &err, &warn, filename);
        }
        else
        {
            res = loader.LoadASCIIFromFile(&tinyModel, &err, &warn, filename);
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
            std::cout << tinyModel << std::endl;
            std::cout << "Buffers : " << tinyModel.buffers.size() << std::endl;
            std::cout << "Buffer Views : " << tinyModel.bufferViews.size() << std::endl;

            GLuint vao;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
        }
        else
        {
            std::cout << "Failed to load glTF: " << filename << std::endl;
        }

        return res;
    }

    bool loadBinary(const char *filename, Model &model)
    {
        return load(filename, true);
    }

    bool loadAscii(const char *filename, Model &model)
    {
        return load(filename, false);
    }

    std::ostream &operator<<(std::ostream &os, const Model &model)
    {
        for (auto &mesh : model.meshes)
        {
            os << "mesh : " << mesh.name << '\n';
            for (auto &primitive : mesh.primitives)
            {
                const Accessor &indexAccessor =
                    model.accessors[primitive.indices];

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
