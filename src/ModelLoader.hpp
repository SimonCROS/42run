#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include "Model.hpp"

#include "glad/gl.h"

#include <optional>

class ModelLoader
{
public:
    bool completed;
    bool error;
    std::optional<tinygltf::Model> model;

    void loadAscii(const char* filename, tinygltf::Model* model, GLuint* vao, std::map<int, GLuint>& buffers, std::map<int, GLuint> &textures);
    void loadBinary(const char* filename, tinygltf::Model* model, GLuint* vao, std::map<int, GLuint>& buffers, std::map<int, GLuint> &textures);

private:
    void load(const char* filename, tinygltf::Model* model, GLuint* vao, std::map<int, GLuint>& buffers, std::map<int, GLuint> &textures);
    void loadAsync(const char* filename, tinygltf::Model* model, GLuint* vao, std::map<int, GLuint>& buffers, std::map<int, GLuint> &textures);
};

std::ostream& operator<<(std::ostream& os, const tinygltf::Model& model);

#endif
