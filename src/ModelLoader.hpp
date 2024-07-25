#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include "Model.hpp"

namespace ModelLoader
{
    bool loadAscii(const char *filename, tinygltf::Model *model);
    bool loadBinary(const char *filename, tinygltf::Model *model);

    std::ostream &operator<<(std::ostream &os, const tinygltf::Model &model);
};

#endif
