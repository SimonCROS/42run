# ---------------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------------

set(EXTERNAL_LIBRARIES_DIR ${CMAKE_SOURCE_DIR}/lib)

set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(GLFW_INCLUDE_NONE OFF CACHE INTERNAL "" FORCE)

set(TINYGLTF3_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(TINYGLTF3_INSTALL OFF CACHE INTERNAL "" FORCE)

set(GLM_ENABLE_CXX_20 ON CACHE BOOL "Enable C++20 features in GLM")
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)

# ---------------------------------------------------------------------------------
# Find OpenGL
# ---------------------------------------------------------------------------------
# FindOpenGL prefers GLVND by default when available
cmake_policy(SET CMP0072 NEW)

find_package(OpenGL 4.1 REQUIRED)

# ---------------------------------------------------------------------------------
# Download or retrieve glfw
# ---------------------------------------------------------------------------------
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG d9d6f0f1f967807ffade6598ea9a631ebaf37a56 #refs/tags/3.5.1
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(glfw)

# ---------------------------------------------------------------------------------
# Download or retrieve tinygltf
# ---------------------------------------------------------------------------------
FetchContent_Declare(
        tinygltf
        GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
        GIT_TAG 0d9ce1a9561688c6c6e7471430d15a50a0b15f1a #refs/tags/v3.0.1
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(tinygltf)

# ---------------------------------------------------------------------------------
# Download or retrieve glm
# ---------------------------------------------------------------------------------
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 8d1fd52e5ab5590e2c81768ace50c72bae28f2ed #refs/tags/1.0.3
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(glm)

# ---------------------------------------------------------------------------------
# Add libraries subdirectories
# ---------------------------------------------------------------------------------
add_subdirectory(${EXTERNAL_LIBRARIES_DIR}/glad EXCLUDE_FROM_ALL)
add_subdirectory(${EXTERNAL_LIBRARIES_DIR}/imgui EXCLUDE_FROM_ALL)
