cmake_minimum_required(VERSION 3.22)
project(stb)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(stb INTERFACE stb_image.h)
target_include_directories(stb INTERFACE .)
