#pragma once

namespace Physics
{
	namespace kernels
	{
		inline float SmoothingPow2(float dist, float radius)
		{
			if (dist < radius)
			{
				float volume = 15 / (2 * glm::pi<float>() * powf(radius, 5));
				float v = radius - dist;
				return v * v * volume;
			}
			return 0;
		}

		inline float SmoothingPow3(float dist, float radius)
		{
			if (dist < radius)
			{
				float volume = 15.0f / (glm::pi<float>() * powf(radius, 6));
				float v = radius - dist;
				return v * v * v * volume;
			}
			return 0;
		}


		inline float SmoothingDerivativePow2(float dist, float radius)
		{
			if (dist <= radius)
			{
				float scale = 15.0f / (powf(radius, 5) * glm::pi<float>());
				float v = (radius - dist);
				return -v * scale;
			}
			return 0;
		}

		inline float SmoothingDerivativePow3(float dist, float radius)
		{
			if (dist <= radius)
			{
				float scale = 45 / (powf(radius, 6) * glm::pi<float>());
				float v = (radius - dist);
				return -v * v * scale;
			}
			return 0;
		}

		inline float SmoothingViscoPoly6(float dist, float radius)
		{
			if (dist < radius)
			{
				float scale = 315 / (64 * glm::pi<float>() * powf(abs(radius), 9));
				float v = radius * radius - dist * dist;
				return v * v * v * scale;
			}
			return 0;
		}
	}
}