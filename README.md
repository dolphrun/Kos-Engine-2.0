# 👾 KosEngine 2.0  

![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![CMake](https://img.shields.io/badge/Build-CMake-green.svg)
![Tests](https://img.shields.io/badge/Testing-GoogleTest-orange.svg)
![Status](https://img.shields.io/badge/Status-Active-success.svg)

KosEngine 2.0 is an ongoing **modular, high-performance 3D game engine** built with **modern C++**, featuring **compile-time reflection**, a **powerful asset system**, **PhysX integration**, and a **robust 3D rendering architecture**. 

This release focuses on **performance, scalability**, and **automated testing** through **Bitbucket Pipelines**.  

---
## Preview
<img width="1898" height="1000" alt="Screenshot 2025-10-26 010148" src="https://github.com/user-attachments/assets/f41733e9-afb0-4b02-86b4-5d602736ff25" />

## 📂 Repository Structure

The repository is organized into several key components:

- **kOS/**: The main engine and editor source code.
    - **Engine/**: Core engine library (ECS, Graphics, Physics, etc.).
    - **Kos Editor/**: The built-in 3D editor application.
    - **Alchemication/**: Application framework and windowing logic.
    - **ScriptingDLL/**: C++ scripting system for game logic.
    - **Test/**: Unit and integration tests using GoogleTest.
- **Compiler Solutions/**: Standalone tools for compiling assets (Fonts, Meshes, Textures, Videos).
- **bin/**: Output directory for compiled binaries and required DLLs.

## 🚀 Features  

### 🧱 Core Architecture  
- **Entity-Component-System (ECS)**: Modular and efficient gameplay logic.
- **Compile-Time Reflection**: For serialization, editor integration, and metadata generation.
- **Field Metadata System**: Enabling safe introspection and manipulation of class fields.
- **C++ Scripting**: Hot-reloadable scripting system for rapid gameplay iteration.

### 🖥️ Graphics  
- **Modern 3D Rendering Pipeline**: Support for materials, meshes, lighting, and cameras.
- **GPU Resource Management**: Optimized GPU resource and draw call management.
- **Shader System**: Built-in support for advanced shaders and materials.

### 📦 Asset & Resource System  
- **Asset Pipeline**: Specialized compilers handled via `Compiler Solutions`.
- **Centralized Asset Manager**: Caching, reference counting, and hot-reloading.
- **JSON-based Definitions**: Easy editing and tool integration for scenes and prefabs.

## ⚙️ Build & Run

### Prerequisites
- **Windows 10/11**
- **CMake** (3.21+)
- **Visual Studio 2022** (recommended for C++ environment)

### Build Instructions
1. In the root folder, run one of the provided batch scripts:
   - `Build&Run-Debug.bat` (for development and debugging)
   - `Build&Run-Release.bat` (for performance testing)
2. These scripts will invoke CMake and build all subprojects into the `bin/` directory.

### Running the Editor
1. Navigate to the binary output directory:
   ```bash
   cd ./kOs/bin/Debug  # or Release
   ```
2. Launch the editor:
   ```bash
   ./Kos_Editor.exe
   ```

## 🧪 Testing  
- **GoogleTest**: Comprehensive unit testing for core systems.
- **CI/CD**: Automated builds and test validation via Bitbucket Pipelines.

