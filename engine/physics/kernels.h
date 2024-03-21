#pragma once

// 
// Copyright 2023 Alexander Marklund (Allkams02@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand associated
// documentation files(the “Software”), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and /or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN 
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

namespace Physics
{
	namespace kernels
	{
		// Density Kernel
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

		// Near Density Kernel
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

		// Pressure Kernel
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

		// Near Pressure Kernel
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

		// Viscosity Kernel
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