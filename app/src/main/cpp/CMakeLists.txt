cmake_minimum_required(VERSION 3.22.1)
project("mcme")

add_library(
    mcme-engine
    SHARED
    jni_bridge.cpp
    engine/Shader.cpp
    engine/Camera.cpp
    engine/Chunk.cpp
    engine/World.cpp
    engine/Renderer.cpp
)

target_include_directories(mcme-engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_library(log-lib log)
find_library(android-lib android)
find_library(gles3-lib GLESv3)
find_library(egl-lib EGL)

target_link_libraries(
    mcme-engine
    ${log-lib}
    ${android-lib}
    ${gles3-lib}
    ${egl-lib}
)

target_compile_options(mcme-engine PRIVATE -std=c++17 -O2 -ffast-math -Wall)
