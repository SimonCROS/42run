add_executable(42run)

target_compile_features(42run PUBLIC
        cxx_std_23
)

target_compile_options(42run PRIVATE -fsanitize=address -g3)
target_link_options(42run PRIVATE -fsanitize=address -g3)

target_sources(42run
        PUBLIC
                Utility/EnumHelpers.h
        PUBLIC
        FILE_SET CXX_MODULES
        FILES
                Components/Components.ixx
                Components/Components_Animator.ixx
                Components/Components_CameraController.ixx
                Components/Components_ImguiSingleton.ixx
                Components/Components_MapController.ixx
                Components/Components_MeshRenderer.ixx
                Components/Components_PlayerController.ixx
                Components/Components_UserInterface.ixx
                Components/Engine_Camera.ixx
                Components/SkyboxRenderer.ixx
                Engine/Animation.ixx
                Engine/AnimationChannel.ixx
                Engine/AnimationSampler.ixx
                Engine/Engine.ixx
                Engine/Engine_Component.ixx
                Engine/Engine_Engine.ixx
                Engine/Engine_Model.ixx
                Engine/Engine_Object.ixx
                Engine/Engine_ObjectsManager.ixx
                Engine/Engine_Transform.ixx
                Engine/FrameInfo.ixx
                Engine/RenderInfo.ixx
                Image.ixx
                InterfaceBlocks/InterfaceBlocks.ixx
                InterfaceBlocks/InterfaceBlocks_AnimationInterfaceBlock.ixx
                InterfaceBlocks/InterfaceBlocks_DisplayInterfaceBlock.ixx
                OpenGL/Buffer/Buffer.ixx
                OpenGL/Buffer/Buffer_Builder.ixx
                OpenGL/Cubemap/Cubemap.ixx
                OpenGL/Cubemap/Cubemap_Builder.ixx
                OpenGL/OpenGL.ixx
                OpenGL/OpenGL_Debug.ixx
                OpenGL/OpenGL_IndicesBuffer.ixx
                OpenGL/OpenGL_VertexArray.ixx
                OpenGL/OpenGL_VertexBuffer.ixx
                OpenGL/Program/Pipeline.ixx
                OpenGL/Program/RenderPass.ixx
                OpenGL/Shader.ixx
                OpenGL/ShaderFile.ixx
                OpenGL/ShaderFlags.ixx
                OpenGL/ShaderManager.ixx
                OpenGL/ShaderProgram.ixx
                OpenGL/Texture2D/Texture2D.ixx
                OpenGL/Texture2D/Texture2D_Builder.ixx
                OpenGL/UniformValue.ixx
                OpenGL2/StateCache.ixx
                OpenGL2/glToString.ixx
                Time.ixx
                Utility/SlotSet.ixx
                Utility/StridedIterator.ixx
                Utility/StringUnorderedMap.ixx
                Utility/Utility.ixx
                Window/Window.ixx
                Window/Window_Context.ixx
                Window/Window_Controls.ixx
                Window/Window_Window.ixx
        PRIVATE
                Components/Components_Animator.cpp
                Components/Components_ImguiSingleton.cpp
                Components/Components_MapController.cpp
                Components/Components_MeshRenderer.cpp
                Components/Components_PlayerController.cpp
                Components/Components_UserInterface.cpp
                Components/Engine_Camera.cpp
                Engine/Animation.cpp
                Engine/AnimationSampler.cpp
                Engine/Engine_Engine.cpp
                Engine/Engine_Model.cpp
                Engine/Engine_Object.cpp
                Engine/Engine_ObjectsManager.cpp
                Engine/Engine_Transform.cpp
                OpenGL/Buffer/Buffer.cpp
                OpenGL/Cubemap/Cubemap.cpp
                OpenGL/Texture2D/Texture2D.cpp
                Window/Window_Context.cpp
                Window/Window_Window.cpp
                main.cpp
                tiny_gltf_impl.cpp
)

target_compile_definitions(42run PRIVATE
        GLFW_INCLUDE_NONE
        GLM_ENABLE_EXPERIMENTAL
        TINYGLTF_NO_STB_IMAGE_WRITE
        TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
        TINYGLTF_USE_CPP14
        STBI_ONLY_JPEG
        STBI_ONLY_PNG
        STBI_ONLY_TGA
        STBI_ONLY_HDR
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
