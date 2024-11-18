#version 430

#define NumThreads 1024

uniform int NumParticles;
uniform float interactionRadius;

layout(std430, binding = 2) buffer BlockPredictedPositions
{
    vec4 PredictedPositions[]; // XYZ
};

layout(std430, binding = 5) buffer BlockSpatialIndices
{
    vec4 SpatialIndices[];  // index, hash, key, null 
};

layout(std430, binding = 6) buffer BlockSpatialOffsets
{
    uint SpatialOffsets[];
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

    SpatialOffsets[id] = 2147483647;

    uint index = id;
    ivec3 cell = GetCell3D(PredictedPositions[id].xyz, interactionRadius);
    int hash = HashCell3D(cell);
    int key = KeyFromHash(hash, NumParticles);
    SpatialIndices[id] = vec4(index, hash, key, 0);
}