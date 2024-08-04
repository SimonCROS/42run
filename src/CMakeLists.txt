# ---------------------------------------------------------------------------------
# Project name
# ---------------------------------------------------------------------------------
set(TARGET_NAME 42run)

# ---------------------------------------------------------------------------------
# Include paths
# ---------------------------------------------------------------------------------
# include_directories(${CMAKE_CURRENT_LIST_DIR} path)

# ---------------------------------------------------------------------------------
# Add source files
# ---------------------------------------------------------------------------------
file(GLOB_RECURSE PROJECT_INCLUDES
        *.hpp
        *.h
)

file(GLOB_RECURSE PROJECT_SOURCES
        *.cpp
        *.cxx
        *.cc
        *.c
)

# define a grouping for files in IDE
source_group("include" FILES ${PROJECT_INCLUDES})
source_group("source" FILES ${PROJECT_SOURCES})

# ---------------------------------------------------------------------------------
# Executable and target include/link libraries
# ---------------------------------------------------------------------------------
add_executable(${TARGET_NAME}
        tiny_gltf_impl.cpp
        main.cpp
        utils/string_unordered_map.hpp
        logger.hpp
        Camera.cpp
        Camera.hpp
        ModelLoader.cpp
        ModelLoader.hpp
        Shader.cpp
        Shader.hpp
        ShaderProgram.cpp
        ShaderProgram.hpp
        ShaderProgramVariants.cpp
        ShaderProgramVariants.hpp
)

target_link_libraries(${TARGET_NAME} PRIVATE glm::glm)
target_link_libraries(${TARGET_NAME} PRIVATE glad)
target_link_libraries(${TARGET_NAME} PRIVATE OpenGL::GL)
target_link_libraries(${TARGET_NAME} PRIVATE glfw)
target_link_libraries(${TARGET_NAME} PRIVATE stb)
target_link_libraries(${TARGET_NAME} PRIVATE tinygltf)
