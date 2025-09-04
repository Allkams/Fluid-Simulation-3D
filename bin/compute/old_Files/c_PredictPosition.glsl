#version 430

#define NumThreads 1024

uniform float TimeStep;
uniform int NumParticles;
uniform float gravityScale;

layout(std430, binding = 0) buffer Positions
{
    vec4 ReadPosAndScale[];
};

layout(std430, binding = 2) buffer BlockPredictedPositions
{
    vec4 PredictedPositions[]; // XYZ
};

layout(std430, binding = 3) buffer BlockVelocities
{
    vec4 Velocities[];  // XYZ
};

layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= NumParticles)
    { 
        return;
    }

    Velocities[id] += vec4(0, -gravityScale, 0, 0) * TimeStep;
    PredictedPositions[id] = vec4(ReadPosAndScale[id].xyz + Velocities[id].xyz * (1.0/120.0), 0.0);
}