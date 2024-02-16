#pragma once

#include <vector>
#include "boundary.h"
#include "spatialStruct.h"

namespace Physics
{
	
	namespace Fluid
	{
		enum Arrangements
		{
			GRID,
			RANDOM,
			CIRCLE,
			size
		};


		//Break this out to another file.
		class FluidSimulation
		{
		public:

			static FluidSimulation& getInstace();

			void Update(float deltatime);

			void InitializeData(int particleAmmount, glm::vec2 Centre = {0,0}, Arrangements type = Arrangements::GRID);

			glm::vec2& getPosition(uint32 particleIndex);
			glm::vec2& getVelocity(uint32 particleIndex);
			float getDensity(uint32 particleIndex);
			float getSpeed(uint32 particleIndex);
			float getSpeedNormalzied(uint32 particleIndex);

		private:

			void updateDensities();

			float CalculateDensity(uint32 particleIndex);
			float ConvertDensityToPressure(float density);
			float CalculateSharedPressure(float densityA, float densityB);

			glm::vec2& CalculatePressureForce(uint32 particleIndex);
			glm::vec2& CalculateViscosityForce(uint32 particleIndex);

			void UpdateSpatialLookup();

			const float interactionRadius = 0.35f;
			const float TargetDensity = 10.0f;
			const float pressureMultiplier = 5.0f;
			const float viscosityStrength = 0.0f;

			bool gravity = false;

			uint32 numParticles;
			std::vector<uint32> pList;

			std::vector<glm::vec2> positions;
			std::vector<glm::vec2> predictedPositions;
			std::vector<glm::vec2> velocity;

			std::vector<float> densities;

			glm::vec2 PositionToCellCoord(const glm::vec2& pos);
			uint32_t HashCell(int X, int Y);
			uint32_t GetKeyFromHash(const uint32_t hash, const uint32_t spatialLength);

			std::vector<SpatialStruct> spatialLookup;
			std::vector<uint32_t> startIndices;

			const glm::vec2 offsets[9] = { {-1,-1}, {0, -1 }, {1, -1}, {-1, 0}, {0,0}, {1, 0}, {-1, 1}, {0,1}, {1,1} };

			Boundary bound;

			void GridArrangement(int rowSize, float gap);
			void RandomArrangement();
			void CircleArrangement();

		private:
			FluidSimulation() {};
			FluidSimulation(const FluidSimulation& cpy) = delete;
			~FluidSimulation() {};
		};
	}

	class PhysicsWorld
	{
	public:

		static PhysicsWorld& getInstace();

		void update(float deltatime);

	private:
		PhysicsWorld() {};
		~PhysicsWorld() {};
		PhysicsWorld(const PhysicsWorld& cpy) = delete;
	};


}