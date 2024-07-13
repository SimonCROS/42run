cmake_minimum_required(VERSION 3.0)
project(StbImage)

add_library(stb_image stb_image.h)
set_target_properties(stb_image PROPERTIES LINKER_LANGUAGE C)
target_include_directories(glad PUBLIC .)
