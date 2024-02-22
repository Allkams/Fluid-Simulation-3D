#include "config.h"
#include "random.h"

namespace Core
{
    union RandomUnion
    {
        unsigned int i;
        float f;
    };

	uint FastRandom()
	{
        static uint x = 123456789;
        static uint y = 362436069;
        static uint z = 521288629;
        static uint w = 88675123;
        uint t;
        t = x ^ (x << 11);
        x = y;
        y = z;
        z = w;
        return w = w ^ (w >> 19) ^ (t ^ (t >> 8));
	}
	float RandomFloatNTP()
	{
        static RandomUnion r;
        r.i = FastRandom() & 0x007fffff | 0x40000000;
        return r.f - 3.0f;
	}
}