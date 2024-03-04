#pragma once

#include <vector>
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

			void InitializeData(int particleAmmount, glm::vec3 Centre = { 0,0 ,0}, Arrangements type = Arrangements::GRID);

			glm::vec3 getPosition(uint32 particleIndex);
			glm::vec3 getVelocity(uint32 particleIndex);
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

			/*void setMousePosition(glm::vec3 pos);
			glm::vec3 getMousePosition();

			void setInputStrength(float value);
			float getInputStrength();*/

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

			float interactionRadius = 0.35f;
			float TargetDensity = 99.9f;
			float pressureMultiplier = 300.0f;
			float nearPressureMultiplier = 20.0f;
			float viscosityStrength = 0.5f;

			float simTime = 0.0f;

			bool gravity = false;
			float gravityScale = 10.0f;

			/*glm::vec3 InteractionMousePoint = {0,0, 0};
			float InteractionInputStrength = 0.0f;
			float InteractionInputRadius = 0.99f;*/

			uint32 numParticles;
			std::vector<uint32> pList;

			std::vector<glm::vec3> predictedPositions;
			std::vector<glm::vec3> velocity;

			std::vector<glm::vec2> densities; // density, neardensity

			glm::vec3 PositionToCellCoord(const glm::vec3& pos);
			uint32_t HashCell(int X, int Y, int Z);
			uint32_t GetKeyFromHash(const uint32_t hash, const uint32_t spatialLength);

			std::vector<SpatialStruct> spatialLookup;
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

			glm::vec3 BoundScale = { 10, 10, 10 };
			glm::mat4 boundTransform = glm::mat4(1);
			glm::quat boundRotation = glm::identity<glm::quat>();

			void GridArrangement(int particlesPerAxel, float gap, const glm::vec3& centre = glm::vec3(0, 0, 0));
			void RandomArrangement(int gap);
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