#version 430

layout(std430, binding = 0) buffer BlockPositions
{
    vec4 PosAndScale[];
}

layout(std430, binding = 1) buffer BlockColors
{
    vec4 Colors[];
}

// layout(std430, binding = 2) buffer BlockVelocities
// {
//     vec4 Velocities[];
// }

uniform uint numParticles;

const int NumThreads = 64;

layout (local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void main() {
    uint id = gl_GlobalInvocationID.x;
    if (id >= numParticles) return;

    // Dispatch the appropriate kernel based on the kernel index
    // You may need to check kernelIndex to determine which kernel to run

    // Example:
    // if (kernelIndex == 0) {
    //     ExternalForces(id);
    // } else if (kernelIndex == 1) {
    //     SpatialHash(id);
    // } else if (kernelIndex == 2) {
    //     Density(id);
    // }
    // Add similar blocks for each kernel

    // ExternalForces(id);
    // SpatialHash(id);
    // Density(id);
    // Pressure(id);
    // Viscosity(id);
    // UpdatePosition(id);
}

UpdatePosition