add_executable(42run
        tiny_gltf_impl.cpp

        Window/Window_Window.cpp
        Window/Window_Window.h
        Window/Controls.cpp
        Window/Controls.h

        OpenGL/VertexBuffer.h
        OpenGL/VertexBuffer.cpp
        OpenGL/VertexArray.cpp
        OpenGL/VertexArray.h
        OpenGL/IndicesBuffer.cpp
        OpenGL/IndicesBuffer.h
        OpenGL/Shader.cpp
        OpenGL/Shader.h
        OpenGL/ShaderProgramInstance.cpp
        OpenGL/ShaderProgramInstance.h
        OpenGL/ShaderProgram.cpp
        OpenGL/ShaderProgram.h
        OpenGL/Debug.cpp
        OpenGL/Debug.h
        OpenGL/Pipeline.cpp
        OpenGL/Pipeline.h

        Components/ImguiSingleton.cpp
        Components/ImguiSingleton.h
        Components/UserInterface.cpp
        Components/UserInterface.h
        Components/CameraController.cpp
        Components/CameraController.h
        Components/MeshRenderer.cpp
        Components/MeshRenderer.h
        Components/Animator.cpp
        Components/Animator.h
        Components/Camera.cpp
        Components/Camera.h

        Engine/Engine.cpp
        Engine/Engine.h
        Engine/AnimationSampler.cpp
        Engine/AnimationSampler.h
        Engine/Animation.cpp
        Engine/Animation.h
        Engine/Mesh.cpp
        Engine/Mesh.h
        Engine/EngineComponent.h
        Engine/Object.cpp
        Engine/Object.h
        Engine/FrameInfo.h
        Engine/Transform.cpp
        Engine/Transform.h

        Utility/EnumHelpers.h
        Utility/StridedIterator.h
        Utility/VectorMultiMap.h
        Utility/Expected.h
        Utility/StringUnorderedMap.h

        InterfaceBlocks/DisplayInterfaceBlock.cpp
        InterfaceBlocks/DisplayInterfaceBlock.h
        InterfaceBlocks/AnimationInterfaceBlock.cpp
        InterfaceBlocks/AnimationInterfaceBlock.h
        InterfaceBlocks/CameraTargetInterfaceBlock.cpp
        InterfaceBlocks/CameraTargetInterfaceBlock.h
        Engine/ObjectsManager.cpp
        Engine/ObjectsManager.h
        Utility/SlotSet.h
        Components/MapController.cpp
        Components/MapController.h
        Components/PlayerController.cpp
        Components/PlayerController.h
)

target_sources(42run
        PUBLIC
                FILE_SET CXX_MODULES
                FILES
                        main.ixx
                        Window/Window.ixx
                        Window/Window_Context.ixx
        PRIVATE
                Window/Window_Context.cpp
)

target_compile_definitions(42run PRIVATE
        GLFW_INCLUDE_NONE
        GLM_ENABLE_EXPERIMENTAL
        TINYGLTF_NO_STB_IMAGE_WRITE
        TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
        TINYGLTF_USE_CPP14
)

target_link_libraries(42run PRIVATE
        glad
        glfw
        glm::glm
        imgui
        OpenGL::GL
        tinygltf
)

target_include_directories(42run PUBLIC
        ${PROJECT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)
