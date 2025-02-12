cmake_minimum_required(VERSION 3.22)
project(Glad)

add_library(glad include/glad/gl.h src/gl.c)
target_include_directories(glad PUBLIC include/)

set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)
