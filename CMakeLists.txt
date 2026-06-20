cmake_minimum_required(VERSION 3.28)
#set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

project(42run VERSION 1.0)

include(FetchContent)
include(${CMAKE_SOURCE_DIR}/cmake/SetupExternalLibraries.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_MODULE_STD ON)

configure_file(42runConfig.h.in 42runConfig.h)
add_subdirectory(src)
