cmake_minimum_required(VERSION 3.28)
project(42run)

set(CMAKE_CXX_STANDARD 17)

#configure_file(42runConfig.h.in 42runConfig.h)

set(EXTERNAL_LIBRARIES_DIR ${CMAKE_SOURCE_DIR}/lib)

include(${CMAKE_SOURCE_DIR}/cmake/SetupExternalLibraries.cmake)

# ---------------------------------------------------------------------------------
# 42run
# ---------------------------------------------------------------------------------
add_subdirectory(src)
