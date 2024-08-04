#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include "Model.hpp"

#include "glad/gl.h"
#include "ShaderProgramVariants.hpp"

#include <iostream>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <atomic>

class ModelLoader
{
public:
    GLuint vao = 0; // TODO Make a class to hold data when loaded
    tinygltf::Model model; // TODO Make a class to hold data when loaded
    std::map<int, GLuint> buffers; // TODO Make a class to hold data when loaded
    std::map<int, GLuint> textures; // TODO Make a class to hold data when loaded
    std::unordered_set<ShaderFlags> usedShaderFlagCombinations; // TODO Make a class to hold data when loaded

    ModelLoader() = delete;
    ModelLoader(const ModelLoader&) = delete;
    ModelLoader(ModelLoader&& other);
    explicit ModelLoader(const std::string_view& filename);

    ModelLoader& operator=(ModelLoader&& other);

    void LoadAsync();
    void Wait();
    void Prepare();
    bool BuildShaders(ShaderProgramVariants& programVariants);
    void Destroy();

    bool IsCompleted();
    bool IsError();
    bool IsBinaryFile() const;

    std::thread loadingThread;
private:
    const std::string _filename;

    std::atomic<bool> error = false;
    std::atomic<bool> completed = false;

    bool LoadWorker();
    void LoadThread();
};

std::ostream& operator<<(std::ostream& os, const tinygltf::Model& model);

#endif
