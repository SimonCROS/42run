cmake_minimum_required(VERSION 3.28)
project(tinygltf)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(tinygltf INTERFACE tiny_gltf.h json.hpp)
target_include_directories(tinygltf INTERFACE .)
