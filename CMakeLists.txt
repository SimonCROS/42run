cmake_minimum_required(VERSION 3.28)
include(FetchContent)

project(42run VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(42runConfig.h.in 42runConfig.h)

set(EXTERNAL_LIBRARIES_DIR ${CMAKE_SOURCE_DIR}/lib)
include(${CMAKE_SOURCE_DIR}/cmake/SetupExternalLibraries.cmake)

add_subdirectory(src)
