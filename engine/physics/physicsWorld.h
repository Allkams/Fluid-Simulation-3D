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

#include <vector>s

namespace Physics
{
	
	namespace Fluid
	{

		class FluidSimulation
		{
		public:

			static FluidSimulation& getInstance();

			void Update(float deltatime);

			void InitializeData(int particleAmmount, glm::vec3 Centre = { 0,0 ,0});

			glm::vec3 getPosition(uint32 particleIndex);
			glm::vec3 getVelocity(uint32 particleIndex);
			float getDensity(uint32 particleIndex);
			float getNearDensity(uint32 particleIndex);
			float getSpeed(uint32 particleIndex);
			float getSpeedNormalzied(uint32 particleIndex);

			double getElapsedTimeGravity();
			double getElapsedTimeSpatial();
			double getElapsedTimeDensity();
			double getElapsedTimePressure();
			double getElapsedTimeViscosity();
			double getElapsedTimePosNColl();

			void setSimulationTime(float time);
			float getSimulationTime();

			void setGravity(bool status);
			bool getGravityStatus();

			void setInteractionRadius(float value);
			float getInteractionRadius();

			void setDensityTarget(float value);
			float getDensityTarget();

			void setPressureMultiplier(float value);
			float getPressureMultiplier();

			void setNearPressureMultiplier(float value);
			float getNearPressureMultiplier();

			void setViscosityStrength(float value);
			float getViscosityStrength();

			void setGravityScale(float value);
			float getGravityScale();

			void setBound(const glm::vec3& value);
			glm::vec3 getBounds();

			std::vector<glm::vec3> positions;
			std::vector<glm::vec4> OutPositions;
		private:

			void updateDensities();

			glm::vec3 CalculateExternalFoce(const glm::vec3& pos, const glm::vec3& vel);

			glm::vec2 CalculateDensity(const glm::vec3& pos);
			float ConvertDensityToPressure(float density);
			float ConvertNearDensityToPressure(float nearDensity);

			void CalculatePressureForce(uint32 particleIndex, float deltatime);
			void CalculateViscosityForce(uint32 particleIndex, float deltatime);

			void UpdateSpatialLookup();

			const float sqrRadius = 0.35f * 0.35f;
			float interactionRadius = 0.35f;
			float TargetDensity = 99.7f;
			float pressureMultiplier = 300.0f;
			float nearPressureMultiplier = 20.0f;
			float viscosityStrength = 0.5f;

			float simTime = 0.0f;

			bool gravity = false;
			float gravityScale = 10.0f;

			double ElapsedTimeGravity = 0.0;
			double ElapsedTimeSpatial = 0.0;
			double ElapsedTimeDensity = 0.0;
			double ElapsedTimePressure = 0.0;
			double ElapsedTimeViscosity = 0.0;
			double ElapsedTimePositionNCollision = 0.0;

			uint32 numParticles;
			std::vector<uint32> pList;

			std::vector<glm::vec3> predictedPositions;
			std::vector<glm::vec3> velocity;
			std::vector<glm::vec3> velocity2;

			std::vector<glm::vec2> densities; // density, neardensity

			glm::vec3 PositionToCellCoord(const glm::vec3& pos);
			uint32_t HashCell(const glm::vec3& inCell);
			uint32_t GetKeyFromHash(const uint32_t hash, const uint32_t spatialLength);

			std::vector<glm::vec3> spatialLookup; // index, hash, key
			std::vector<uint32_t> startIndices;

			const glm::vec3 offsets[27] = { 
				{-1, -1, -1}, {-1, -1, 0}, {-1, -1, 1}, 
				{-1, 0, -1}, {-1, 0, 0}, {-1, 0, 1},
				{-1, 1, -1}, {-1, 1, 0}, {-1, 1 ,1},

				{0, -1, -1}, {0, -1, 0}, {0, -1, 1},
				{0, 0, -1}, {0, 0, 0}, {0, 0, 1},
				{0, 1, -1}, {0, 1, 0}, {0, 1, 1},

				{1, -1, -1}, {1, -1, 0}, {1, -1, 1},
				{1, 0, -1}, {1, 0, 0}, {1, 0, 1},
				{1, 1, -1}, {1, 1, 0}, {1, 1, 1}
			};

			glm::vec3 BoundScale = { 20, 20, 20 };
			glm::mat4 boundTransform = glm::mat4(1);
			glm::quat boundRotation = glm::identity<glm::quat>();

			void GridArrangement(int particlesPerAxel, float gap, const glm::vec3& centre = glm::vec3(0, 0, 0));

		private:
			FluidSimulation() {};
			FluidSimulation(const FluidSimulation& cpy) = delete;
			~FluidSimulation() {};
		};
	}

	inline bool compareByKey(const glm::vec3& a, const glm::vec3& b)
	{
		return a.z < b.z;
	}
}