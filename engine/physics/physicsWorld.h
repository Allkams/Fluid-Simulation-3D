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

			void InitializeData(int particleAmmount, glm::vec2 Centre = { 0,0 }, Arrangements type = Arrangements::GRID);

			glm::vec2 getPosition(uint32 particleIndex);
			glm::vec2 getVelocity(uint32 particleIndex);
			float getDensity(uint32 particleIndex);
			float getNearDensity(uint32 particleIndex);
			float getSpeed(uint32 particleIndex);
			float getSpeedNormalzied(uint32 particleIndex);

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

			void setBound(const glm::vec2& value);
			glm::vec2 getBounds();

		private:

			void updateDensities();

			glm::vec2 CalculateDensity(const glm::vec2& pos);
			float ConvertDensityToPressure(float density);
			float ConvertNearDensityToPressure(float nearDensity);

			void CalculatePressureForce(uint32 particleIndex, float deltatime);
			void CalculateViscosityForce(uint32 particleIndex, float deltatime);

			void UpdateSpatialLookup();

			float interactionRadius = 0.35f;
			float TargetDensity = 45.0f;
			float pressureMultiplier = 20.0f;
			float nearPressureMultiplier = 2.0f;
			float viscosityStrength = 0.05f;

			float simTime = 0.0f;

			bool gravity = false;
			float gravityScale = 9.82f;

			uint32 numParticles;
			std::vector<uint32> pList;

			std::vector<glm::vec2> positions;
			std::vector<glm::vec2> predictedPositions;
			std::vector<glm::vec2> velocity;

			std::vector<glm::vec2> densities; // density, neardensity

			glm::vec2 PositionToCellCoord(const glm::vec2& pos);
			uint32_t HashCell(int X, int Y);
			uint32_t GetKeyFromHash(const uint32_t hash, const uint32_t spatialLength);

			std::vector<SpatialStruct> spatialLookup;
			std::vector<uint32_t> startIndices;

			const glm::vec2 offsets[9] = { {-1,-1}, {0, -1 }, {1, -1}, {-1, 0}, {0,0}, {1, 0}, {-1, 1}, {0,1}, {1,1} };

			glm::vec2 Bound = { 16,9 };

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