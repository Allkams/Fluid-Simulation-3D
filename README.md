# SPH Fluid Simulation 3D
This is a repository for my 3D version of a SPH Langarian fluid simulation using my own rendering engine [SpectraWise-Engine](https://github.com/Allkams/SpectraWise-Engine). The implementation is using OpenGL 4.6.  
My 2D implementation version can be found here: [Fluid-Simulation-2D](https://github.com/Allkams/Fluid-Simulation-2D)  
  
> [!NOTE]
> The simulation is utilizing Multithreading and GPU Billboard rendering.

## Table of Content
- [Requirements](#requirements)
- [Instructions](#instructions)
- [TODO List](#todo-list)
- [Bug Reporting](#bug-reporting)
    - [Known Issues](#known-issues)
- [Resarch Papers and referenses](#resarch-papers-and-referenses)
- [Examples](#examples)
- [External Librarys](#external-librarys)
- [External Help](#external-librarys)
    - [CMAKE](#cmake)
    - [OpenGL](#opengl) 

## Requirements
- Cmake 3.2+
- Compiler that runs C++20

## Instructions
Download the repo to you computer. Generate the build file with CMake and just go crazy!
All of the Fluid simulation files is in the Engine->Physics folder.
> [!NOTE]
> A more descriptive manual is [found here](https://github.com/Allkams/Fluid-Simulation-3D/blob/main/MANUAL.md)


## TODO List
- [X] Cleanup code!
- [ ] Make Compute shader work
- [ ] Make 3D bound better looking
- [ ] Apply stickyness to the particles to mimic water better.
- [ ] Implement a simple Rigidbody to particle collision system
- [ ] Experiment with Spheretrace rendering for water.
- [ ] Implement simple lights
- [ ] Make water reflective.

## Bug Reporting  
Would you encounter any bugs within the simulation, please create an issue and report it!  

### Known Issues
 - [ ] Viscosity infinity energy loop. (Making infinity swirls in corners)
 - [ ] Compute shader implementation not working.

## Resarch Papers and referenses  
 - [Particle-based Viscoelastic Fluid Simulation](http://www.ligum.umontreal.ca/Clavet-2005-PVFS/pvfs.pdf)
 - [Real-Time Fluid Dynamics for Games](http://graphics.cs.cmu.edu/nsp/course/15-464/Fall09/papers/StamFluidforGames.pdf)
 - [Particle-based Fluid Simulation for Interactive Applications](https://matthias-research.github.io/pages/publications/sca03.pdf)
 - [Smoothed Particle HydroDynamics Part 3](https://arxiv.org/pdf/1007.1245.pdf)
 - [Particle Simulation using CUDA](https://web.archive.org/web/20140725014123/https://docs.nvidia.com/cuda/samples/5_Simulations/particles/doc/particles.pdf)
 - [Smoothed Particle Hydrodynamics techniques for the physics based simulation of fluids and solids](https://sph-tutorial.physics-simulation.org/pdf/SPH_Tutorial.pdf)
 - [Sebastian Lauge Fluid Simulation, Coding adventure](https://www.youtube.com/watch?v=rSKMYc1CQHE)
 - [SPlisHSPlasH](https://github.com/InteractiveComputerGraphics/SPlisHSPlasH)

## Examples
Loading Examples ... 

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
