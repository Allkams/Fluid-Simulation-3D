#version 430

#define NumThreads 1024
#define M_PI 3.1415926535897932384626433832795
//Includes
// #include "shaders/spatialHash.glsl"
// #include "shaders/FluidMath.glsl"

// NOTE: Math things

float DensityKernel(float dist, float radius)
{
    if(dist < radius)
    {
        float volume = 15 / (2 * M_PI * pow(radius, 5));
        float v = radius - dist;
        return v * v * volume;
    }
    return 0;
}

float NearDensityKernel(float dist, float radius)
{
    if(dist < radius)
    {
        float volume = 15 / (2 * M_PI * pow(radius, 6));
        float v = radius - dist;
        return v * v * v * volume;
    }
    return 0;
}

float DensityDerivativeKernal(float dist, float radius)
{
    if (dist <= radius)
    {
        float scale = 15 / (pow(radius, 5) * M_PI);
        float v = (radius - dist);
        return -v * scale;
    }
    return 0;
}

float NearDensityDerivativeKernal(float dist, float radius)
{
    if (dist <= radius)
    {
        float scale = 45 / (pow(radius, 6) * M_PI);
        float v = (radius - dist);
        return -v * v * scale;
    }
    return 0;
}

float SmoothingViscoPoly6(float dist, float radius)
{
    if (dist < radius)
    {
        float scale = 315 / (64 * M_PI * pow(abs(radius), 9));
        float v = radius * radius - dist * dist;
        return v * v * v * scale;
    }
    return 0;
}

// NOTE: SpatialHash things
const vec3 offsets3D[27] =
{
	vec3(-1, -1, -1),
	vec3(-1, -1, 0),
	vec3(-1, -1, 1),
	vec3(-1, 0, -1),
	vec3(-1, 0, 0),
	vec3(-1, 0, 1),
	vec3(-1, 1, -1),
	vec3(-1, 1, 0),
	vec3(-1, 1, 1),
	vec3(0, -1, -1),
	vec3(0, -1, 0),
	vec3(0, -1, 1),
	vec3(0, 0, -1),
	vec3(0, 0, 0),
	vec3(0, 0, 1),
	vec3(0, 1, -1),
	vec3(0, 1, 0),
	vec3(0, 1, 1),
	vec3(1, -1, -1),
	vec3(1, -1, 0),
	vec3(1, -1, 1),
	vec3(1, 0, -1),
	vec3(1, 0, 0),
	vec3(1, 0, 1),
	vec3(1, 1, -1),
	vec3(1, 1, 0),
	vec3(1, 1, 1)
};

// Constants used for hashing
#define hashK1 15823
#define hashK2 9737333
#define hashK3 440817757

// Convert floating point position into an integer cell coordinate
vec3 GetCell3D(vec3 position, float radius)
{
	vec3 cell = floor(position / radius);
    return vec3(int(cell.x), int(cell.y), int(cell.z));
}

// Hash cell coordinate to a single unsigned integer
uint HashCell3D(vec3 cell)
{
	return (int(cell.x) * hashK1) + (int(cell.y) * hashK2) + (int(cell.z) * hashK3);
}

uint KeyFromHash(uint hash, uint tableSize)
{
	return hash % tableSize;
}
// -----------------------------------------------------------------------------------------------------------

//Buffers
layout(std430, binding = 0) buffer Positions
{
    vec4 ReadPosAndScale[];
};

layout(std430, binding = 1) buffer BlockColor
{
    vec4 ReadColors[];
};

layout(std430, binding = 2) buffer BlockPredictedPositions
{
    vec4 PredictedPositions[]; // XYZ
};

layout(std430, binding = 3) buffer BlockVelocities
{
    vec4 Velocities[];  // XYZ
};

layout(std430, binding = 4) buffer BlockDensities
{
    // NOTE: (FarDensity , NearDensity)
    vec2 Densities[]; 
};

layout(std430, binding = 5) buffer BlockSpatialIndices
{
    vec4 SpatialIndices[];  // index, hash, key, null 
};

layout(std430, binding = 6) buffer BlockSpatialOffsets
{
    uint SpatialOffsets[];
};


//Settings
uniform float TimeStep;
uniform int NumParticles;

uniform float interactionRadius;
uniform float targetDensity;
uniform float pressureMultiplier;
uniform float nearPressureMultiplier;
uniform float viscosityStrength;
uniform float gravityScale;

uniform vec3 boundSize;
uniform vec3 centre;
uniform vec4 Color1;
uniform vec4 Color2;
uniform vec4 Color3;
uniform vec4 Color4;



void ResolveCollisions(uint particleIndex)
{
    const float dampFactor = 0.95;
    const vec3 halfSize = boundSize * 0.5;
    vec3 edgeDst = halfSize - abs(ReadPosAndScale[particleIndex].xyz);

    if (edgeDst.x <= 0)
    {
        ReadPosAndScale[particleIndex].x = halfSize.x * sign(ReadPosAndScale[particleIndex].x);
        Velocities[particleIndex].x *= -1 * dampFactor;
    }

    if (edgeDst.y <= 0)
    {
        ReadPosAndScale[particleIndex].y = halfSize.y * sign(ReadPosAndScale[particleIndex].y);
        Velocities[particleIndex].y *= -1 * dampFactor;
    }

    if (edgeDst.z <= 0)
    {
        ReadPosAndScale[particleIndex].z = halfSize.z * sign(ReadPosAndScale[particleIndex].z);
        Velocities[particleIndex].z *= -1 * dampFactor;
    }

}

// NOTE: GPU SORT

uint groupWidth;
uint groupHeight;
uint StepIndex;

layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void Sort()
{
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
}

layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void bitonicSort() {
    uint tid = gl_GlobalInvocationID.x;

    for (uint k = 2; k <= NumParticles; k <<= 1) {
        for (uint j = k >> 1; j > 0; j >>= 1) {
            uint ixj = tid ^ j;

            if (ixj > tid) {
                bool ascending = ((tid & k) == 0);
                vec4 d1 = SpatialIndices[tid];
                vec4 d2 = SpatialIndices[ixj];

                if ((d1.z > d2.z) == ascending) {
                    SpatialIndices[tid] = d2;
                    SpatialIndices[ixj] = d1;
                }
            }

            barrier();
        }
    }
}

layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void calculateOffsets()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= NumParticles) return;

    uint key = uint(SpatialIndices[id].z);
    uint keyPrev = id == 0 ? 4294967295 : uint(SpatialIndices[id-1].z);
    if(key != keyPrev)
    {
        SpatialOffsets[key] = id;
    }

}

// NOTE: END OF GPU SORT

layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void UpdateColors()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= NumParticles)
    { 
        return;
    }

    float speedNormal = clamp(length(Velocities[id]), 0.0, 1.5) / 1.5;

    float breakpoint1 = 0.33; // 33% of the gradient
    float breakpoint2 = 0.66; // 66% of the gradient

    // Calculate the colors based on the normalized value
    if (speedNormal <= breakpoint1) {
        ReadColors[id] = (1.0f - speedNormal / breakpoint1) * Color1 + (speedNormal / breakpoint1) * Color2;
    }
    else if (speedNormal <= breakpoint2) {
        ReadColors[id] = (1.0f - (speedNormal - breakpoint1) / (breakpoint2 - breakpoint1)) * Color2 +
            ((speedNormal - breakpoint1) / (breakpoint2 - breakpoint1)) * Color3;
    }
    else {
        ReadColors[id] = (1.0f - (speedNormal - breakpoint2) / (1.0f - breakpoint2)) * Color3 +
            ((speedNormal - breakpoint2) / (1.0f - breakpoint2)) * Color4;
    }
}

layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void ExternalForces()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= NumParticles)
    { 
        return;
    }

    Velocities[id] += vec4(0, -gravityScale, 0, 0) * TimeStep;
    PredictedPositions[id] = vec4(ReadPosAndScale[id].xyz + Velocities[id].xyz * (1.0/120.0), 0.0);
}

layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void SpatialHashUpdate()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= NumParticles) return;

    SpatialOffsets[id] = 2147483647;

    uint index = id;
    vec3 cell = GetCell3D(PredictedPositions[id].xyz, interactionRadius);
    uint hash = HashCell3D(cell);
    uint key = KeyFromHash(hash, NumParticles);
    SpatialIndices[id] = vec4(index, hash, key, 0);
    // Will not work... Will need a GPU Sort funciton for spatialofffsets to align..
    // TODO: Implement a Bitonic sort.
}

layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void CalculateDensities()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= NumParticles) return;

    bool debug = id == 0 ? true : false;

    vec3 pos = PredictedPositions[id].xyz;
    vec3 originCell = GetCell3D(pos, interactionRadius);
    float sqrRadius = interactionRadius * interactionRadius;
    float density = 0;
    float nearDensity = 0;

    // for(int i = 0; i < NumParticles; i++)
    // {
    //     vec3 neighbourPos = PredictedPositions[i].xyz;
    //     vec3 offsetToNeighbour = neighbourPos - pos;
    //     float sqrDstToNeighbour = dot(offsetToNeighbour, offsetToNeighbour);

    //     if(sqrDstToNeighbour > sqrRadius)
    //     {
    //         continue;
    //     }

    //     float dst = sqrt(sqrDstToNeighbour);
    //     density += DensityKernel(dst, interactionRadius);
    //     nearDensity += NearDensityKernel(dst, interactionRadius);
    //     if(debug)
    //     {
    //         ReadColors[i] = vec4(1.0,0,0,1.0);
    //     }
    // }

    for(int i = 0; i < 27; i++)
    {
        uint hash = HashCell3D(originCell + offsets3D[i]);
        uint key = KeyFromHash(hash, NumParticles);
        uint currIndex = SpatialOffsets[key]; //FIXME: NOT WORKING, NO SORTER.
        while (currIndex < NumParticles)
        {
            vec3 indexData = SpatialIndices[currIndex].xyz;
            currIndex++;

            if(uint(indexData.z) != key) break;
            if(uint(indexData.y) != hash) continue;

            uint neighbourIndex = uint(indexData.x);
            vec3 neighbourPos = PredictedPositions[neighbourIndex].xyz;
            vec3 offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = dot(offsetToNeighbour, offsetToNeighbour);


            if(sqrDstToNeighbour > sqrRadius)
            {
                continue;
            }

            float dst = sqrt(sqrDstToNeighbour);
            density += DensityKernel(dst, interactionRadius);
            nearDensity += NearDensityKernel(dst, interactionRadius);

            if(debug)
            {
                ReadColors[neighbourIndex] = vec4(1.0,0,0,1.0);
                ReadColors[id] = vec4(0.0,1,1,1.0);
            }
        }
    }
    //vec3 debugColor = vec3(density / targetDensity);
    //ReadColors[id] = vec4(debugColor, 1.0);
    Densities[id] = vec2(density, nearDensity);
}

float pressureFromDensity(float density)
{
    return (density - targetDensity) * pressureMultiplier;
}

float nearPressureFromDensity(float density)
{
    return density * nearPressureMultiplier;
}

layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void CalculatePressureForce()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= NumParticles) return;

    float density = Densities[id].x;
    float nearDensity = Densities[id].y;
    float pressure = pressureFromDensity(density);
    float nearPressure = nearPressureFromDensity(nearDensity);
    vec3 pressureForce = vec3(0);

    vec3 pos = PredictedPositions[id].xyz;
    vec3 originCell = GetCell3D(pos, interactionRadius);
    float sqrRadius = interactionRadius * interactionRadius;

    // for(int i = 0; i < NumParticles; i++)
    // {
    //     uint neighbourIndex = uint(i);
    //     if(neighbourIndex == id) continue;

    //     vec3 neighbourPos = PredictedPositions[neighbourIndex].xyz;
    //     vec3 offsetToNeighbour = neighbourPos - pos;
    //     float sqrDstToNeighbour = dot(offsetToNeighbour, offsetToNeighbour);

    //     if (sqrDstToNeighbour > sqrRadius) continue;

    //     float densityNeighbour = Densities[neighbourIndex].x;
    //     float nearDensityNeighbour = Densities[neighbourIndex].y;
    //     float neighbourPressure = pressureFromDensity(densityNeighbour);
    //     float nearNeighbourPressure = nearPressureFromDensity(nearDensityNeighbour);

    //     float sharedPressure = (pressure + neighbourPressure) / 2;
    //     float sharedNearPressure = (nearPressure + nearNeighbourPressure) / 2;

    //     float dst = sqrt(sqrDstToNeighbour);
    //     vec3 dir = dst > 0 ? offsetToNeighbour / dst : vec3(0,1,0);

    //     pressureForce += dir * DensityDerivativeKernal(dst, interactionRadius) * sharedPressure / densityNeighbour;
    //     pressureForce += dir * NearDensityDerivativeKernal(dst, interactionRadius) * sharedNearPressure / nearDensityNeighbour;

    // }
    for (int i = 0; i < 27; i++)
    {
        uint hash = HashCell3D(originCell + offsets3D[i]);
        uint key = KeyFromHash(hash, NumParticles);
        uint currIndex = SpatialOffsets[key]; //FIXME: NOT WORKING, NO SORTER.

        while (currIndex < NumParticles)
        {
            vec3 indexData = SpatialIndices[currIndex].xyz;
            currIndex++;

            if (uint(indexData.z) != key) break;
            if (uint(indexData.y) != hash) continue;

            uint neighbourIndex = uint(indexData.x);
            if(neighbourIndex == id) continue;

            vec3 neighbourPos = PredictedPositions[neighbourIndex].xyz;
            vec3 offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = dot(offsetToNeighbour, offsetToNeighbour);

            if (sqrDstToNeighbour > sqrRadius) continue;

            float densityNeighbour = Densities[neighbourIndex].x;
            float nearDensityNeighbour = Densities[neighbourIndex].y;
            float neighbourPressure = pressureFromDensity(densityNeighbour);
            float nearNeighbourPressure = nearPressureFromDensity(nearDensityNeighbour);

            float sharedPressure = (pressure + neighbourPressure) / 2;
            float sharedNearPressure = (nearPressure + nearNeighbourPressure) / 2;

            float dst = sqrt(sqrDstToNeighbour);
            vec3 dir = dst > 0 ? offsetToNeighbour / dst : vec3(0,1,0);

            pressureForce += dir * DensityDerivativeKernal(dst, interactionRadius) * sharedPressure / densityNeighbour;
            pressureForce += dir * NearDensityDerivativeKernal(dst, interactionRadius) * sharedNearPressure / nearDensityNeighbour;
        }
    }
    vec3 acceleration = pressureForce / density;
    Velocities[id] += vec4(acceleration * TimeStep, 0.0);
}

layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void CalculateViscosity()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= NumParticles) return;

    vec3 pos = PredictedPositions[id].xyz;
    vec3 originCell = GetCell3D(pos, interactionRadius);
    float sqrRadius = interactionRadius * interactionRadius;
    vec3  viscosityForce = vec3(0);
    vec3 velocity = Velocities[id].xyz;

    // for(int i = 0; i < NumParticles; i++)
    // {
    //     uint neighbourIndex = i;
    //     if(neighbourIndex == id) continue;
    //     vec3 neighbourPos = PredictedPositions[neighbourIndex].xyz;
    //     vec3 offsetToNeighbour = neighbourPos - pos;
    //     float sqrDstToNeighbour = dot(offsetToNeighbour, offsetToNeighbour);

    //     if (sqrDstToNeighbour > sqrRadius) continue;

    //     float dst = sqrt(sqrDstToNeighbour);
    //     vec3 neighbourVelocity = Velocities[neighbourIndex].xyz;
    //     viscosityForce += (neighbourVelocity - velocity) * SmoothingViscoPoly6(dst, interactionRadius);
    // }

    for (int i = 0; i < 27; i++)
    {
        uint hash = HashCell3D(originCell + offsets3D[i]);
        uint key = KeyFromHash(hash, NumParticles);
        uint currIndex = SpatialOffsets[key]; //FIXME: NOT WORKING, NO SORT
        while (currIndex < NumParticles)
        {
            vec3 indexData = SpatialIndices[currIndex].xyz;
            currIndex++;

            if (uint(indexData.z) != key) break;
            if (uint(indexData.y) != hash) continue;

            uint neighbourIndex = uint(indexData.x);
            if(neighbourIndex == id) continue;
            vec3 neighbourPos = PredictedPositions[neighbourIndex].xyz;
            vec3 offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = dot(offsetToNeighbour, offsetToNeighbour);

            if (sqrDstToNeighbour > sqrRadius) continue;

            float dst = sqrt(sqrDstToNeighbour);
            vec3 neighbourVelocity = Velocities[neighbourIndex].xyz;
            viscosityForce += (neighbourVelocity - velocity) * SmoothingViscoPoly6(dst, interactionRadius);
        }
    }
    Velocities[id] += vec4(viscosityForce * viscosityStrength * TimeStep, 0.0);
}

layout (local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void UpdatePosition()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= NumParticles) return;

    ReadPosAndScale[id] += vec4(Velocities[id].xyz * TimeStep, 0);
    ResolveCollisions(id);
}

int nextPowerOfTwo(uint n) {
    uint power = 1;
    while (power < n) {
        power <<= 1;
    }
    return int(power);
}

//layout (local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void main() 
{
    // Dispatch Kernerls
    //ExternalForces();
    //barrier();
    //SpatialHashUpdate();
    //barrier();
    //bitonicSort();
    //barrier();
    //calculateOffsets();
    //barrier();
    //CalculateDensities();
    //barrier();
    //CalculatePressureForce();
    //barrier();
    //CalculateViscosity();
    //barrier();
    UpdatePosition();
    //UpdateColors();
}