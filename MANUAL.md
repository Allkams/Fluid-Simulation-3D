# Fluid Simulation 3D Manual
This file contains all the valuable information about what you need to install and run the simulation.
> [!CAUTION]  
> The project is CPU-intensive. We have simulated 50,000 particles on an AMD Threadripper in Release mode, achieving 50-60 fps.
> Depending on your hardware, managing this number of particles may be challenging. The system has a minimum cap of 2 fps, and frame rates below 40 fps may result in buggy simulations for large particle sizes.

> [!NOTE]  
> The project has only been tested on Windows 10 and 11.

## Installation guide
The project is very simple to get started with. Most libraries and dependencies are already in the project files.
But for a list on [Dependencies](#dependencies) and [external Libraries](#libraries), click their link or scroll down till you find their section.

### Compilers and build systems
To get started you need both Visual Studio and CMake 3.2 or newer. Make sure the Visual studio version can handle C++ 20.
- Visual Studio Download link: [Click Here](https://visualstudio.microsoft.com/downloads/)  
- CMake Download link: [Click Here](https://cmake.org/download/)  

With these two installed we can start the installation process.

### Installation
1. Download the repository.
    - Downloading a zip and extract it
    - Use a command prompt and fetch the repository
    - Download the repository through github desktop
2. Build the project with CMake.
    - **(2.1)** Select the repository folder on "Where is the source code".
    - **(2.2)** Paste the following to "Wher to build the binaries": "/Your-Path-To-Repo/Build"
    - **(2.3)** Click Configure, In the second popup window select "Visual Studio 17 2022" as your generator, leave "Use default native comilers" as it is.
    - **(2.4)** Click Finish.
    - **(2.5)** If the text "Configuring done" is visible in the output box, click on Generate.
    - **(2.6)** Now the project is built!
3. Open the Build folder
4. Double Click on the FluidSim.sln to open the project in visual studio.
5. In visual studio: Right click on the "simulation", and select "Select as startup project".
6. Change the particle amount in "gameApp.cc" file, there you should find the line `#define particleAmount 50000`, change 50 000 to the desired amount.
7. For better performace, change "Debug" to "Release" in the top section of visual studios UI.
8. Press F5 or the button "Local Windows Debugger" to start the project

## Project Usage
When project installed and working its time to use the UI and make kaos with the simulation!

### UI Menu: Debug INFO
This menu give you all information you need to see the performace of the system.
- **FPS:** Displays the current frames per seconds. This can not go lower than 2 fps.
- **Number of Particles:** As the name states, this displays the selected amount of particles to simulate.
- **PROGRAM DATA:** This collapsable header contains technical data that describes the performace of the system. Most parts of the code is encapsulated with a timer which this utilizes.
    - **Simulation Status:** Shows if the simulation is running or not.
    - **Simulation Elapsed:** The time the update function for the fluid/liquid simulation takes per frame.
    - **Rendering Elapse:** The time it takes to render all particles. This is very low as this only counts the offloading to the gpu.
    - **Color Elapsed:** How long does it take to change and update the color? Well, this displays that time.
    - **Program Elapsed:** This is the deltasecond that the whole program takes to run. Taking 1/deltasecond will result in fps, 1/fps will result in deltasecond.
    - **SIMULATION DATA:** This collapsable header contains technical data that describes the performace of each function in the simulation.
- **PARTICLE DATA:** This is the second collapsable header and this contains information about specific particles. This includes a selection of a particle index and the shows the position, density, and velocity for that particle. No rendering have though been implemented to show this selected particle..

### UI Menu: Values
In this menu you can manage all depending values of the simulation. Here you take control over the simulation and it looks.
- **START/STOP:** Here you can start, stop and reset the simulation.
- **Gravity:** This boolean (true/false) value make it posible to toggle gravity on and off.
- **Interaction Radius:** This is the surrounding sphere that desides the volume of interaction for the particles. This also changes the [smoothing kernels](https://github.com/Allkams/Fluid-Simulation-3D/blob/main/engine/physics/kernels.h) as they act from the interaction radius and distance between particles.
- **Target Density:** This is the density the system/simulation will strive against.
- **Pressure multiplier:** The multiplier that decides how much effect the pressure should have. The bigger the value, the more pressure applied.
- **Near Pressure Multiplier:** Following the double density relaxsation principles from Smooth Particle Hydrodynamics we apply a near density and a near pressure to more drastic make sure we never overlap particles.
- **Viscosity Strength:** This deside how thick the liquid should be, the closer to one, the more tension there is between particles, the lower value, the more spread the liquid will be.
- **Gravity Scale:** Here we decide how much gravity should affect the particles. This alone could remove the gravity boolean.
- **Bounding Volume:** As this is not a open world simulation, this value desides the volume and area the simulation can be inside.
- **Colors:** This collapsable header have 4 collapsable headers inside of it. Each color gives the value for a gradient. Color 1 0%, Color2 33%, Color3 66%, Color4 100%. This makes debugging easier!

## Dependencies
- CMake 3.2
- C++ 20
- OpenGL 4.6

## Libraries
- GLFW
- GLEW
- Dear ImGUI
- NanoVG
- Soloud
- GLM
