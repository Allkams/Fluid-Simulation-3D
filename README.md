# SPH Fluid Simulation 3D
A Smooth-Particle Hydrodynamic(SPH) Langarian fluid simulation project. Stared as my specialization project for my final year of "Computer Game Programming" at Luleå University of Technology in 2024, now being a hobby project which I update occacionally.

This project focuses on understanding the fundamentals of fluid simulations in various scenarios, with the aim of developing a robust system capable of pre-calculating realistic simulations for animations. By leveraging Universal Scene Descriptor (USD), the project will import scenes from 3D software, simulate fluid behavior with accurate collision handling, and export real-time simulations.
Additionally, the project serves as a platform for learning and experimentation. It aims to explore optimization techniques, track the impact of evolving hardware capabilities, and document the journey of improvement over time. The ultimate goal is to maintain a well-structured, evolving project that aligns with both technical growth and personal learning milestones.

The project uses my own rendering engine [SpectraWise-Engine](https://github.com/Allkams/SpectraWise-Engine) as its base. For more engine status, documentation, and specifics follow the link. Both projects is updated side by side and new dependencies or features might not be listed here.  
An 2D implementation version I made with "[The PlayBuffer](https://github.com/sumo-digital-academy/playbuffer)" can be found here: [Fluid-Simulation-2D](https://github.com/Allkams/Fluid-Simulation-2D).

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
- [ ] Make bound rotate.
- [ ] Possibillity to export and import to 3D softwares like Houdini.
- [ ] Add liquid presets
- [ ] Implement a usable UI
- [ ] Add better movement
- [ ] Implement input manager
- [ ] Implement quick reset of shaders
- [ ] Add possibilities to change from 2D to 3D
- [ ] Add possibilities to change from CPU to GPU Calculations
- [ ] Add possibility to import and export Universal Scene Description(USD) files.
- [X] Dynamic color gradient in water.

## Bug Reporting  
Would you encounter any bugs within the simulation, please create an issue and report it!  

### Known Issues
 - [ ] Viscosity infinity energy loop. (Making infinity swirls in corners)
 - [ ] Compute shader implementation not working.
 - [ ] Pressure going crazy when the fps is below 40.

## Resarch Papers and referenses  
 - [Particle-based Viscoelastic Fluid Simulation](http://www.ligum.umontreal.ca/Clavet-2005-PVFS/pvfs.pdf)
 - [Real-Time Fluid Dynamics for Games](http://graphics.cs.cmu.edu/nsp/course/15-464/Fall09/papers/StamFluidforGames.pdf)
 - [Particle-based Fluid Simulation for Interactive Applications](https://matthias-research.github.io/pages/publications/sca03.pdf)
 - [Smoothed Particle HydroDynamics Part 3](https://arxiv.org/pdf/1007.1245.pdf)
 - [Particle Simulation using CUDA](https://web.archive.org/web/20140725014123/https://docs.nvidia.com/cuda/samples/5_Simulations/particles/doc/particles.pdf)
 - [Smoothed Particle Hydrodynamics techniques for the physics based simulation of fluids and solids](https://sph-tutorial.physics-simulation.org/pdf/SPH_Tutorial.pdf)
 - [Sebastian Lauge Fluid Simulation, Coding adventure (video)](https://www.youtube.com/watch?v=rSKMYc1CQHE)
 - [SPlisHSPlasH](https://github.com/InteractiveComputerGraphics/SPlisHSPlasH)
 - [OpenUSD](https://openusd.org/docs/)

## Examples
Loading Examples ... 

## External Librarys
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
