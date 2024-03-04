#version 430

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
	return (vec3)floor(position / radius);
}

// Hash cell coordinate to a single unsigned integer
uint HashCell3D(vec3 cell)
{
	return ((int)cell.x * hashK1) + ((int)cell.y * hashK2) + ((int)cell.z * hashK3);
}

uint KeyFromHash(uint hash, uint tableSize)
{
	return hash % tableSize;
}