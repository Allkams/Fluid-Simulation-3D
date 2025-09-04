#version 430

#define NumThreads 1024
#define M_PI 3.1415926535897932384626433832795

uniform int NumParticles;
uniform float interactionRadius;

layout(std430, binding = 1) buffer BlockColor
{
    vec4 ReadColors[];
};

layout(std430, binding = 2) buffer BlockPredictedPositions
{
    vec4 PredictedPositions[]; // XYZ
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


// NOTE: SpatialHash things
const ivec3 offsets3D[27] =
{
	ivec3(-1, -1, -1),
	ivec3(-1, -1, 0),
	ivec3(-1, -1, 1),
	ivec3(-1, 0, -1),
	ivec3(-1, 0, 0),
	ivec3(-1, 0, 1),
	ivec3(-1, 1, -1),
	ivec3(-1, 1, 0),
	ivec3(-1, 1, 1),
	ivec3(0, -1, -1),
	ivec3(0, -1, 0),
	ivec3(0, -1, 1),
	ivec3(0, 0, -1),
	ivec3(0, 0, 0),
	ivec3(0, 0, 1),
	ivec3(0, 1, -1),
	ivec3(0, 1, 0),
	ivec3(0, 1, 1),
	ivec3(1, -1, -1),
	ivec3(1, -1, 0),
	ivec3(1, -1, 1),
	ivec3(1, 0, -1),
	ivec3(1, 0, 0),
	ivec3(1, 0, 1),
	ivec3(1, 1, -1),
	ivec3(1, 1, 0),
	ivec3(1, 1, 1)
};

// Constants used for hashing
#define hashK1 15823
#define hashK2 9737333
#define hashK3 440817757
#define prime 2147483647


// Convert floating point position into an integer cell coordinate
ivec3 GetCell3D(vec3 position, float radius)
{
	vec3 cell = floor(position / radius);
    return ivec3(cell);
}

// Hash cell coordinate to a single unsigned integer
int HashCell3D(ivec3 cell)
{
	return ((cell.x * hashK1) % prime 
    + (cell.y * hashK2) % prime 
    + (cell.z * hashK3) % prime) % prime;
}

int KeyFromHash(int hash, int tableSize)
{
	return hash % tableSize;
}
layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= NumParticles) return;

    bool debug = id == 0 ? true : false;

    vec3 pos = PredictedPositions[id].xyz;
    ivec3 originCell = GetCell3D(pos, interactionRadius);
    float sqrRadius = interactionRadius * interactionRadius;
    float density = 0;
    float nearDensity = 0;

    for(int i = 0; i < 27; i++)
    {
        int hash = HashCell3D(originCell + offsets3D[i]);
        uint key = KeyFromHash(hash, NumParticles);
        uint currIndex = SpatialOffsets[key];
        if(debug)
        {
            ReadColors[id] = vec4(0,1,1,1.0);
        }
        while (currIndex < NumParticles)
        {
            vec3 indexData = SpatialIndices[currIndex].xyz;
            currIndex++;

            if(debug)
            {
                ReadColors[id] = vec4(0,1,0,1.0);
            }
            if(uint(indexData.z) != key) break;
            if(debug)
            {
                ReadColors[id] = vec4(1,1,0,1.0);
            }
            if(uint(indexData.y) != hash) continue;

            uint neighbourIndex = uint(indexData.x);
            vec3 neighbourPos = PredictedPositions[neighbourIndex].xyz;
            vec3 offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = dot(offsetToNeighbour, offsetToNeighbour);

            if(debug)
            {
                ReadColors[id] = vec4(0,0,0,1.0);
            }
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