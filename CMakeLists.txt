cmake_minimum_required(VERSION 3.28)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")

project(42run VERSION 1.0)

include(FetchContent)
include(${CMAKE_SOURCE_DIR}/cmake/SetupExternalLibraries.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_MODULE_STD ON)

configure_file(42runConfig.h.in 42runConfig.h)
add_subdirectory(src)
