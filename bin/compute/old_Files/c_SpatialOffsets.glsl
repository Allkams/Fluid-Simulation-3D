#version 430

#define NumThreads 128

uniform int NumParticles;

layout(std430, binding = 5) buffer BlockSpatialIndices
{
    vec4 SpatialIndices[];  // index, hash, key, null 
};

layout(std430, binding = 6) buffer BlockSpatialOffsets
{
    uint SpatialOffsets[];
};


layout(local_size_x = NumThreads, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= NumParticles) return;

    uint i = id;
	uint null = NumParticles;

	uint key = uint(SpatialIndices[i].z);
	uint keyPrev = i == 0 ? null : uint(SpatialIndices[i - 1].z);

	if (key != keyPrev)
	{
		SpatialOffsets[key] = i;
	}

    // uint key = uint(SpatialIndices[id].z);
    // uint keyPrev = id == 0 ? 4294967295 : uint(SpatialIndices[id-1].z);
    // if(key != keyPrev)
    // {
    //     SpatialOffsets[key] = id;
    // }
}