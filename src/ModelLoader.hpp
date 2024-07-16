#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "tiny_gltf.h"

#include "Model.hpp"

namespace ModelLoader
{
    bool loadAscii(const char *filename, Model &model);
    bool loadBinary(const char *filename, Model &model);

    std::ostream &operator<<(std::ostream &os, const tinygltf::Model &model);
};

#endif
