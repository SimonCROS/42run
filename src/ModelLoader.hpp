#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include "Model.hpp"

#include "glad/gl.h"

namespace ModelLoader
{
    bool loadAscii(const char* filename, tinygltf::Model* model, GLuint* vao, std::map<int, GLuint>& buffers, std::map<int, GLuint> &textures);
    bool loadBinary(const char* filename, tinygltf::Model* model, GLuint* vao, std::map<int, GLuint>& buffers, std::map<int, GLuint> &textures);

    std::ostream& operator<<(std::ostream& os, const tinygltf::Model& model);
};

#endif
