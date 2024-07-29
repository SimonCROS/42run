#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include "Model.hpp"

#include "glad/gl.h"

#include <iostream>
#include <thread>

class ModelLoader
{
public:
    GLuint vao = 0; // TODO Make a class to hold data when loaded
    tinygltf::Model model; // TODO Make a class to hold data when loaded
    std::map<int, GLuint> buffers; // TODO Make a class to hold data when loaded
    std::map<int, GLuint> textures; // TODO Make a class to hold data when loaded

    ModelLoader() = delete;
    ModelLoader(const ModelLoader&) = delete;
    explicit ModelLoader(const std::string_view& filename);

    void LoadAsync();
    void Prepare();

    bool IsCompleted();
    bool IsError();
    bool IsBinaryFile() const;

private:
    const std::string _filename;

    bool error = false;
    bool completed = false;
    std::mutex loadingMutex;
    std::thread loading_thread;

    bool LoadWorker();
    void LoadThread();
};

std::ostream& operator<<(std::ostream& os, const tinygltf::Model& model);

#endif
