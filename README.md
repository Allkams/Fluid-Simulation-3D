# SPH Fluid Simulation 3D
This is a repository for my 3D version of a SPH Langarian fluid simulation using my own rendering engine [SpectraWise-Engine](https://github.com/Allkams/SpectraWise-Engine). The implementation is using OpenGL 4.6.  
My 2D implementation version can be found here: [Fluid-Simulation-2D](https://github.com/Allkams/Fluid-Simulation-2D)

## Table of Content
- [Requirements](#table-of-content)
- [Personal Usage Info](#personal-usage-info)
    - [Instructions](#instructions)
- [Bug Reporting](#bug-reporting)
    - [Known Issues](#known-issues)
- [Examples](#examples)
- [External Librarys](#external-librarys)
- [External Help](#external-librarys)
    - [CMAKE](#cmake)
    - [OpenGL](#opengl) 

## Requirements
- Cmake 3.2+
- Compiler that runs C++20

## Personal Usage Info
If you want to use this engine to build a game or just play around, your free to do!

### Instructions
To make a game or a rendering project you need to add your code in a folder inside of [projects](/projects). This new folder must include its own CMakeList.txt file, which will generate the project and solution. Execute CMake from the root directory, and subsequently open the solution in your intended destination folder.

## Bug Reporting
Should you encounter any bugs within the engine, please create an issue to report it!

### Known Issues
 - [] None

## Examples
Right now no Examples are provided as this is an early stage of development!

## External Librarys
Spectrawise uses:
- GLFW
- GLEW
- Dear ImGUI
- NanoVG
- Soloud
- GLM
<!--## Contribution
Maybe add 

-->
## External help
### CMake
[CMAKE Tutorials](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)

### OpenGL
[LearnOpenGL](https://learnopengl.com/)
