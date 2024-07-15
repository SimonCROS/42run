cmake_minimum_required(VERSION 3.22)

# set the project name and version
project(42run VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# FindOpenGL prefers GLVND by default when available
cmake_policy(SET CMP0072 NEW)

configure_file(42runConfig.h.in 42runConfig.h)

find_package(X11)
if(APPLE AND X11_FOUND)
  set(CMAKE_FIND_FRAMEWORK NEVER)
  find_package(OpenGL REQUIRED)
  unset(CMAKE_FIND_FRAMEWORK)
else()
  find_package(OpenGL REQUIRED)
endif()
find_package(glfw3 3.3 REQUIRED)

add_subdirectory(lib/glad)

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)

add_executable(42run src/main.cpp src/Shader.cpp src/ShaderProgram.cpp)

target_link_libraries(42run PRIVATE glfw)
target_link_libraries(42run PRIVATE OpenGL::GL)
target_link_libraries(42run PRIVATE glm::glm)
target_link_libraries(42run PRIVATE glad)

target_include_directories(42run PUBLIC ${PROJECT_BINARY_DIR})
target_include_directories(42run PUBLIC ${PROJECT_SOURCE_DIR}/lib/stb)
target_include_directories(42run PUBLIC ${PROJECT_SOURCE_DIR}/lib/tinygltf)
