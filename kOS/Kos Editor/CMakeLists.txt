cmake_minimum_required(VERSION 3.16)
project(Kos_Editor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT TARGET Kos_Engine)
    message(FATAL_ERROR "Kos_Engine target not found. Make sure Engine/ is built first.")
endif()


file(GLOB_RECURSE EDITOR_SOURCES
    Application/*.cpp
    AssetManager/*.cpp
    Dependencies/imgui/*.cpp
    Dependencies/imguinodeeditor/*.cpp
    Configs/*.cpp
    Editor/*.cpp
)

add_executable(Kos_Editor ${EDITOR_SOURCES})
add_dependencies(Kos_Editor SCRIPTS)

target_link_libraries(Kos_Editor PRIVATE Kos_Engine)

target_include_directories(Kos_Editor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/imgui"
        "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/imguinodeeditor"
)

if(MSVC)
    target_compile_options(Kos_Editor PRIVATE /Zc:preprocessor)
endif()

