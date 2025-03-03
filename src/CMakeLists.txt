add_executable(42run
        tiny_gltf_impl.cpp
        Utility/EnumHelpers.h
)

target_sources(42run
        PUBLIC
                FILE_SET CXX_MODULES
                FILES
                        Utility/Utility.ixx
                        Utility/Utility_SlotSet.ixx
                        Utility/Utility_StridedIterator.ixx
                        Utility/Utility_StringUnorderedMap.ixx
                        Utility/Utility_VectorMultiMap.ixx
                        Window/Window.ixx
                        Window/Window_Context.ixx
                        Window/Window_Controls.ixx
                        Window/Window_Window.ixx
                        OpenGL/OpenGL.ixx
                        OpenGL/OpenGL_Debug.ixx
                        OpenGL/OpenGL_IndicesBuffer.ixx
                        OpenGL/OpenGL_Pipeline.ixx
                        OpenGL/OpenGL_Shader.ixx
                        OpenGL/OpenGL_ShaderProgram.ixx
                        OpenGL/OpenGL_ShaderProgramInstance.ixx
                        OpenGL/OpenGL_VertexArray.ixx
                        OpenGL/OpenGL_VertexBuffer.ixx
                        Components/Components.ixx
                        Components/Components_Animator.ixx
                        Components/Components_CameraController.ixx
                        Components/Components_ImguiSingleton.ixx
                        Components/Components_MapController.ixx
                        Components/Components_MeshRenderer.ixx
                        Components/Components_PlayerController.ixx
                        Components/Components_UserInterface.ixx
                        Components/Engine_Camera.ixx
                        Engine/Engine.ixx
                        Engine/Engine_Animation.ixx
                        Engine/Engine_AnimationSampler.ixx
                        Engine/Engine_Component.ixx
                        Engine/Engine_Engine.ixx
                        Engine/Engine_FrameInfo.ixx
                        Engine/Engine_Mesh.ixx
                        Engine/Engine_Object.ixx
                        Engine/Engine_ObjectsManager.ixx
                        Engine/Engine_Transform.ixx
                        InterfaceBlocks/InterfaceBlocks.ixx
                        InterfaceBlocks/InterfaceBlocks_AnimationInterfaceBlock.ixx
                        InterfaceBlocks/InterfaceBlocks_DisplayInterfaceBlock.ixx
        PRIVATE
                main.cpp
                Window/Window_Context.cpp
                Window/Window_Window.cpp
                OpenGL/OpenGL_ShaderProgram.cpp
                OpenGL/OpenGL_ShaderProgramInstance.cpp
                Components/Components_Animator.cpp
                Components/Components_ImguiSingleton.cpp
                Components/Components_MapController.cpp
                Components/Components_MeshRenderer.cpp
                Components/Components_PlayerController.cpp
                Components/Components_UserInterface.cpp
                Components/Engine_Camera.cpp
                Engine/Engine_Animation.cpp
                Engine/Engine_AnimationSampler.cpp
                Engine/Engine_Engine.cpp
                Engine/Engine_Mesh.cpp
                Engine/Engine_Object.cpp
                Engine/Engine_ObjectsManager.cpp
                Engine/Engine_Transform.cpp
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
