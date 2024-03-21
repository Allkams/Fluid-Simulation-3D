# Fluid Simulation 3D Manual
This file contains all the valuable information about what you need to install and run the simulation.
> [!CAUTION]  
> The project is CPU-intensive. We have simulated 50,000 particles on an AMD Threadripper (Version coming) and an Nvidia GeForce RTX (Version) in Release mode, achieving 50-60 fps.
> Depending on your hardware, managing this number of particles may be challenging. The system has a minimum cap of 2 fps, and frame rates below 40 fps may result in buggy simulations for large particle sizes.

> [!NOTE]  
> The project has only been tested on Windows 10 and 11.

## Installation guide
The project is very simple to get started with. Most libraries and dependencies are already in the project files.
But for a list on [Dependencies](#dependencies) and [external Libraries](#libraries), click the link on them or scroll down till you find the header.

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
8. Press F5 or the button "Local Windows Debugger" to start the simulation

## Dependencies
Lorem ipsum

## Libraries
Lorem ipusm
