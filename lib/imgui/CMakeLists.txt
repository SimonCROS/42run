cmake_minimum_required(VERSION 3.22)
project(imgui)

add_library(imgui
        imgui_demo.cpp
        imgui_tables.cpp
        imstb_truetype.h
        imgui_draw.cpp
        imgui_widgets.cpp
        imconfig.h
        imgui.h
        imstb_rectpack.h
        imgui.cpp
        imgui_internal.h
        imstb_textedit.h
        backends/imgui_impl_glfw.cpp
        backends/imgui_impl_opengl3.cpp
        backends/imgui_impl_opengl3_loader.h
        backends/imgui_impl_glfw.h
        backends/imgui_impl_opengl3.h
        misc/cpp/imgui_stdlib.h
        misc/cpp/imgui_stdlib.cpp
)

target_include_directories(imgui PUBLIC .)
target_include_directories(imgui PUBLIC backends/)
target_include_directories(imgui PUBLIC misc/cpp/)

target_link_libraries(imgui PRIVATE glfw)
