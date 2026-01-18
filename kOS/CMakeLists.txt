cmake_minimum_required(VERSION 3.21)
project(Kos_Solution LANGUAGES CXX)

set(CMAKE_PDB_OUTPUT_DIRECTORY     "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")

# Global settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
    set(CMAKE_BUILD_PARALLEL_LEVEL 0) # auto-detect CPU cores
endif()


# Global compiler warnings
if(MSVC)
	add_compile_options(/W3 /bigobj)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -pedantic -Wshadow -Wconversion)
endif()

# Subprojects
add_subdirectory(Engine)
add_subdirectory(ScriptingDLL)
add_subdirectory("Kos Editor")
add_subdirectory("Alchemication")
add_subdirectory(Test)

#add_subdirectory(ScriptingDLL)

