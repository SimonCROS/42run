cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

include(FetchContent)

project(42run VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_MODULE_STD ON)

configure_file(42runConfig.h.in 42runConfig.h)

## Dummy target for CLion
#add_library(unused_std_target STATIC)
#target_sources(unused_std_target
#        PRIVATE
#        FILE_SET CXX_MODULES
#        BASE_DIRS /opt/homebrew/Cellar/llvm/19.1.7_1/share/libc++/v1
#        FILES /opt/homebrew/Cellar/llvm/19.1.7_1/share/libc++/v1/std.cppm /opt/homebrew/Cellar/llvm/19.1.7_1/share/libc++/v1/std.compat.cppm)

set(EXTERNAL_LIBRARIES_DIR ${CMAKE_SOURCE_DIR}/lib)
include(${CMAKE_SOURCE_DIR}/cmake/SetupExternalLibraries.cmake)

add_subdirectory(src)
