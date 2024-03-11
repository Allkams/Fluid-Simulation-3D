#pragma once

namespace Physics
{
	struct SpatialStruct
	{
		uint32_t key;
		uint32_t hash;
		uint32_t index;
		SpatialStruct() : key(UINT32_MAX), index(UINT32_MAX), hash(UINT32_MAX) {};
		SpatialStruct(uint32_t inKey, uint32_t inHash, uint32_t inIndex) : key(inKey), index(inIndex), hash(inHash) {};
	};

	inline bool compareByKey(const glm::vec3 & a, const glm::vec3& b)
	{
		return a.z < b.z;
	}
}