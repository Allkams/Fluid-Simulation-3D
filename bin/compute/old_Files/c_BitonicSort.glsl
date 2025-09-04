#version 430

#define NumThreads 128

uniform int NumParticles;
uniform uint groupWidth;
uniform uint groupHeight;
uniform uint StepIndex;
// uniform int stage;
// uniform int passOfStage;

layout(std430, binding = 5) buffer BlockSpatialIndices
{
    vec4 SpatialIndices[];  // index, hash, key, null 
};

layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void main() {

    uint id = gl_GlobalInvocationID.x;
    
    uint i = id;

    uint hIndex = i & (groupWidth - 1);
    uint indexLeft = hIndex + (groupHeight + 1) * (i / groupWidth);
    uint rightStepSize = StepIndex == 0 ? groupHeight - 2 * hIndex : (groupHeight + 1) / 2;
    uint indexRight = indexLeft + rightStepSize;

    if(indexRight >= NumParticles) return;

    uint valueLeft = uint(SpatialIndices[indexLeft].z);
    uint valueRight = uint(SpatialIndices[indexRight].z);

    if(valueLeft > valueRight)
    {
        vec4 temp = SpatialIndices[indexLeft];
        SpatialIndices[indexLeft] = SpatialIndices[indexRight];
        SpatialIndices[indexRight] = temp;
    }


    // uint gid = gl_GlobalInvocationID.x;

    // uint pairDistance = 1 << passOfStage; // Distance between pairs being compared
    // uint blockSize = 1 << stage;         // Current bitonic sequence size
    // uint blockStart = gid & ~(blockSize - 1); // Start of the current bitonic block
    // uint partner = gid ^ pairDistance;       // Index of the partner element

    // if (partner > gid) {
    //     // Determine sort order based on stage
    //     bool ascending = ((gid & blockSize) == 0);
        
    //     // Compare and swap
    //     if ((ascending && SpatialIndices[gid].z > SpatialIndices[partner].z) ||
    //         (!ascending && SpatialIndices[gid].z < SpatialIndices[partner].z)) {
    //         // Swap
    //         vec4 temp = SpatialIndices[gid];
    //         SpatialIndices[gid] = SpatialIndices[partner];
    //         SpatialIndices[partner] = temp;
    //     }
    // }
}