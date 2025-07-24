# BC Engine

The BC Engine is a personal Game Engine project which was derived from my other "Louron Engine" project. This is a total re-approach and re-write to my OpenGL based engine, now using Vulkan. 

My aim has been to re-design and clean up the fundamental design of my old engine to be cleaner, less prone to errors/bugs, and for easier readability. There is still a long way to go...

This project utilises a variety of open source libraries as outlined below.

This project utilises the C++ 2020 standard.

# Building

This project utilises CMAKE for its project management toolchain. 

## Pre-Requisites for Build
The following are requirements for this project to compile on your Windows or Linux machine.
1. A Valid C++ Compiler
    - MSVC for windows (must include ATL/MFC when installing MSBuild Tools)
    - GCC/Clang for Linux
2. CMAKE
    - At least version 3.16
3. Vulkan SDK
    - With VMA headers included

## Build Scripts
Once you have installed the aforementioned pre-requisites, you can build the project by using the scripts contained in ```Tools/{ Win | Linux }```.

You should run the configure script first to configure the CMAKE project, followed by either build, or build_debug script. Once you have successfully built the project, you can run it easily using the run or run_debug scripts.

## VS Code Integration

I have created standard Launch and Task JSON configurations for VS Code which will enable you to build and run directly from VS Code without messing around with the scripts directly.

The Launch options include debug and release configurations for both Windows and Linux.

When launching the debug configuration through VS Code, the live debugger will be active for you to set breakpoints, view call stack/memory, and step through the code.

## MSVC

I believe using MSVC you can open the root BC-Engine folder as a CMAKE project directly through MSVC. This will allow you to build and run the application directly through MSVC.

## Other IDE's

I have not tested with other IDEs, so use the scripts as outlined earlier to build and run the application.

# Features

1. Entity Component System using EnTT
    - Variety of basic components
    - Hierarchy system
    - Transform system
2. Debugging Tools
    - Assertions
    - Logging
    - Scope Profiler
    - Thread Pool Profiler
3. Job System Thread Pool
4. PhysX Implementation
5. Project System
6. Multi-Scene System
7. Octree Spatial Partitioning (for frustum culling and spatial queries)
8. Scripting Manager
    - Utilises C++
    - Loads Assembly during runtime and loads reflection information of Script Classes & factor/release functions.
    - Registers Host Functions to Script Core Engine Callbacks
    - Serialisation/Deserialisation of Script Fields
9. Simple BC Editor GUI application

## WIP 

1. Finish Vulkan Core
2. Finish Scene Renderer
3. Finish Asset Manager
4. Finish Asset Implementations (meshes, textures, materials, shaders, etc.)
5. Implement remaining functionality for BC Editor application
6. Implement Memory Management - fix a memory leak somewhere in the code. I'm unsure where it is but there is a steady memory growth somewhere. I need to create a better approach to memory management for the entire engine.
5. A lot more that I can't think of right now...

TLDR; the remaining functionality currently available in my Louron Engine project, then will start at building more functionality.

# Dependencies

1. Assimp
2. EFSW
3. EnTT
4. GLFW
5. GLM
6. Dear ImGui
    - ImGui Node Editor
    - ImGuizmo
7. Miniaudio
8. spdlog
9. STB Image
10. TinyFileDialogs
11. YAML-CPP
12. PhysX
    - As this library does not have a native approach to using CMAKE, it can not be incorporated to my CMAKE toolchain. I have cloned and compiled the necessary binaries and have uploaded to this repo using GIT LFS for Windows/Linux debug and release builds.
13. clang-c & clang++ binary
    - This is used by the Editor Application to traverse the script project AST, and find all scripts that should be compiled and generate reflection information for the ScriptCore assembly compilation. 
    - This also includes the clang++ exectuable to compile the ScriptCore project. I am considering changing the ScriptCore compilation to a CMAKE project to utilise the system installed compiler, requiring users of the BC  Editor to have a valid C++ 2020 compiler installed on their machines as a pre-requisite.

# Licence

See LICENCE.md for licensing.