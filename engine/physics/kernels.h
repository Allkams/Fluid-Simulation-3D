#pragma once

namespace Physics
{
	namespace kernels
	{
		inline float SmoothingPow2(float dist, float radius)
		{
			if (dist < radius)
			{
				float volume = 6 / (glm::pi<float>() * pow(radius, 4));
				float v = radius - dist;
				return v * v * volume;
			}
			return 0;
		}

		inline float SmoothingPow3(float dist, float radius)
		{
			if (dist < radius)
			{
				float volume = 10.0f / (glm::pi<float>() * pow(radius, 5));
				float v = radius - dist;
				return v * v * v * volume;
			}
			return 0;
		}


		inline float SmoothingDerivativePow2(float dist, float radius)
		{
			if (dist <= radius)
			{
				float scale = 12 / (pow(radius, 4) * glm::pi<float>());
				float v = (radius - dist);
				return -v * scale;
			}
			return 0;
		}

		inline float SmoothingDerivativePow3(float dist, float radius)
		{
			if (dist <= radius)
			{
				float scale = 30 / (pow(radius, 5) * glm::pi<float>());
				float v = (radius - dist);
				return -v * v * scale;
			}
			return 0;
		}

		inline float SmoothingViscoPoly6(float dist, float radius)
		{
			if (dist < radius)
			{
				float scale = 4.0f / (glm::pi<float>() * pow(radius, 8));
				float v = radius * radius - dist * dist;
				return v * v * v * scale;
			}
			return 0;
		}
	}
}