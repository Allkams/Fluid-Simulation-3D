#include "config.h"
#include "physicsWorld.h"

#include "kernels.h"
#include "core/random.h"

#include <chrono>
#include <thread>
#include <execution>

namespace Physics
{
	PhysicsWorld& PhysicsWorld::getInstace()
	{
		static PhysicsWorld instance;

		return instance;
	}

	void PhysicsWorld::update(float deltatime)
	{
		Fluid::FluidSimulation::getInstace().Update(deltatime);
	}
}

namespace Physics
{
	namespace Fluid
	{
		FluidSimulation& FluidSimulation::getInstace()
		{
			static FluidSimulation instance;

			return instance;
		}

		void FluidSimulation::Update(float deltatime)
		{
			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this, deltatime](uint32_t i)
			{
				if (gravity)
				{
					velocity[i] += CalculateExternalFoce(positions[i], velocity[i]) * deltatime;
					//velocity[i].y -= gravityScale * deltatime;
				}
				predictedPositions[i] = positions[i] + velocity[i] * (1.0f / 120.0f);
				//WriteIndex = readIndex;
			});

			UpdateSpatialLookup();

			updateDensities();

			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this, deltatime](uint32_t i)
			{
					CalculatePressureForce(i, deltatime);
			});

			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this, deltatime](uint32_t i)
			{
					CalculateViscosityForce(i, deltatime);
			});

			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this, deltatime](uint32_t i)
			{
				positions[i] += deltatime * velocity[i];

				const float dampFactor = 0.95f;
				const glm::vec2 halfSize = Bound * 0.5f;
				glm::vec2 edgeDst = halfSize - abs(positions[i]);

				if (edgeDst.x <= 0)
				{
					positions[i].x = halfSize.x * glm::sign(positions[i].x);
					velocity[i].x *= -1 * dampFactor;
				}
				if (edgeDst.y <= 0)
				{
					positions[i].y = halfSize.y * glm::sign(positions[i].y);
					velocity[i].y *= -1 * dampFactor;
				}

			});
		}
		void FluidSimulation::InitializeData(int particleAmmount, glm::vec2 Centre, Arrangements type)
		{
			numParticles = particleAmmount;

			if (positions.size() > 0)
			{
				positions.clear();
				velocity.clear();
				predictedPositions.clear();
				densities.clear();
				pList.clear();
			}

			for (int i = 0; i < particleAmmount; i++)
			{
				pList.push_back(i);
			}

			for (int i = 0; i < particleAmmount; i++)
			{
				positions.push_back({ 0,0 });
				velocity.push_back({ 0,0 });
				predictedPositions.push_back({ 0,0 });
				densities.push_back({ 0,0 });
			}

			int RowSize = ceil(glm::sqrt(particleAmmount));
			float gap = 0.115f;
			switch (type)
			{
			case Physics::Fluid::GRID:
				GridArrangement(RowSize, gap);
				break;
			case Physics::Fluid::RANDOM:
				RandomArrangement(gap);
				break;
			case Physics::Fluid::CIRCLE:
				break;
			case Physics::Fluid::size:
				break;
			default:
				break;
			}
			UpdateSpatialLookup();
			updateDensities();

		}

		glm::vec2 FluidSimulation::getPosition(uint32 particleIndex)
		{
			if (particleIndex >= numParticles) return glm::zero<glm::vec2>();
			return positions[particleIndex];
		}

		glm::vec2 FluidSimulation::getVelocity(uint32 particleIndex)
		{
			if (particleIndex >= numParticles) return glm::zero<glm::vec2>();
			return velocity[particleIndex];
		}

		float FluidSimulation::getDensity(uint32 particleIndex)
		{
			if (particleIndex >= numParticles) return 0.0f;
			return densities[particleIndex].x;
		}
		float FluidSimulation::getNearDensity(uint32 particleIndex)
		{
			if (particleIndex >= numParticles) return 0.0f;
			return densities[particleIndex].y;
		}

		float FluidSimulation::getSpeed(uint32 particleIndex)
		{
			if (particleIndex >= numParticles) return 0.0f;
			return glm::length(velocity[particleIndex]);
		}

		float FluidSimulation::getSpeedNormalzied(uint32 particleIndex)
		{
			if (particleIndex >= numParticles) return 0.0f;
			return glm::clamp(glm::length(velocity[particleIndex]), 0.0f, 1.5f) / 1.5f;
		}

		void FluidSimulation::setSimulationTime(float time)
		{
			simTime = time;
		}

		float FluidSimulation::getSimulationTime()
		{
			return simTime;
		}

		void FluidSimulation::setGravity(bool status)
		{
			gravity = status;
		}

		bool FluidSimulation::getGravityStatus()
		{
			return gravity;
		}

		void FluidSimulation::setInteractionRadius(float value)
		{
			interactionRadius = value;
		}

		float FluidSimulation::getInteractionRadius()
		{
			return interactionRadius;
		}

		void FluidSimulation::setDensityTarget(float value)
		{
			TargetDensity = value;
		}

		float FluidSimulation::getDensityTarget()
		{
			return TargetDensity;
		}

		void FluidSimulation::setPressureMultiplier(float value)
		{
			pressureMultiplier = value;
		}

		float FluidSimulation::getPressureMultiplier()
		{
			return pressureMultiplier;
		}

		void FluidSimulation::setNearPressureMultiplier(float value)
		{
			nearPressureMultiplier = value;
		}

		float FluidSimulation::getNearPressureMultiplier()
		{
			return nearPressureMultiplier;
		}

		void FluidSimulation::setViscosityStrength(float value)
		{
			viscosityStrength = value;
		}

		float FluidSimulation::getViscosityStrength()
		{
			return viscosityStrength;
		}

		void FluidSimulation::setGravityScale(float value)
		{
			gravityScale = value;
		}

		float FluidSimulation::getGravityScale()
		{
			return gravityScale;
		}

		void FluidSimulation::setMousePosition(glm::vec2 pos)
		{
			InteractionMousePoint = pos;
		}

		glm::vec2 FluidSimulation::getMousePosition()
		{
			return InteractionMousePoint;
		}

		void FluidSimulation::setBound(const glm::vec2& value)
		{
			Bound = value;
		}

		glm::vec2 FluidSimulation::getBounds()
		{
			return Bound;
		}

		void FluidSimulation::updateDensities()
		{
			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this](uint32_t i)
			{
				densities[i] = CalculateDensity(predictedPositions[i]);
			});
		}

		glm::vec2 FluidSimulation::CalculateExternalFoce(const glm::vec2& pos, const glm::vec2& vel)
		{
			glm::vec2 gravityAccel = { 0, -gravityScale };

			if (InteractionInputStrength != 0)
			{
				glm::vec2 PointOffset = InteractionMousePoint - pos;
				float sqrDist = dot(PointOffset, PointOffset);
				float sqrRadius = InteractionInputRadius * InteractionInputRadius;
				if (sqrDist < sqrRadius)
				{
					float dst = sqrt(sqrDist);
					float edgeT = (dst / InteractionInputRadius);
					float centreT = 1 - edgeT;
					glm::vec2 dirToCentre = PointOffset / dst;

					float gravityWeight = 1 - (centreT * glm::clamp(InteractionInputStrength / 10, 0.0f, 1.0f));
					glm::vec2 accel = gravityAccel * gravityWeight + dirToCentre * centreT * InteractionInputStrength;
					accel -= vel * centreT;
					return accel;
				}
			}

			return gravityAccel;
		}

		glm::vec2 FluidSimulation::CalculateDensity(const glm::vec2& pos)
		{
			glm::vec2 originCell = PositionToCellCoord(pos);
			float sqrRadius = interactionRadius * interactionRadius;
			float density = 0;
			float NearDensity = 0;

			for (int i = 0; i < 9; i++)
			{
				uint32_t hash = HashCell(originCell.x + offsets[i].x, originCell.y + offsets[i].y);
				uint32_t key = GetKeyFromHash(hash, numParticles);
				uint32 currIndex = startIndices[key];

				while (currIndex < numParticles)
				{
					SpatialStruct index = spatialLookup[currIndex];
					currIndex++;
					if (index.key != key) break;


					if (index.hash != hash) continue;

					if (index.index >= numParticles) break;


					uint32_t neighborIndex = index.index;

					glm::vec2 neighbourPos = predictedPositions[neighborIndex];
					glm::vec2 offsetToNeighbour = neighbourPos-pos;
					float sqrDist = dot(offsetToNeighbour, offsetToNeighbour);

					if (sqrDist > sqrRadius) continue;

					float dist = sqrt(sqrDist);
					density += kernels::SmoothingPow2(dist, interactionRadius);
					NearDensity += kernels::SmoothingPow3(dist, interactionRadius);
				}
			}

			return { density, NearDensity };
		}

		float FluidSimulation::ConvertDensityToPressure(float density)
		{
			return (density - TargetDensity) * pressureMultiplier;
		}

		float FluidSimulation::ConvertNearDensityToPressure(float nearDensity)
		{
			return nearDensity * nearPressureMultiplier;
		}

		void FluidSimulation::CalculatePressureForce(uint32 particleIndex, float deltatime)
		{
			float density = densities[particleIndex].x;
			float nearDensity = densities[particleIndex].y;
			float pressure = ConvertDensityToPressure(density);
			float nearPressure = ConvertNearDensityToPressure(nearDensity);
			glm::vec2 pressureForce = { 0,0 };

			glm::vec2 pos = predictedPositions[particleIndex];
			glm::vec2 originCell = PositionToCellCoord(pos);
			float sqrRadius = interactionRadius * interactionRadius;

			for (int i = 0; i < 9; i++)
			{
				uint32_t hash = HashCell(originCell.x + offsets[i].x, originCell.y + offsets[i].y);
				uint32_t key = GetKeyFromHash(hash, numParticles);
				int currIndex = startIndices[key];

				while (currIndex < numParticles)
				{
					SpatialStruct index = spatialLookup[currIndex];
					currIndex++;
					if (index.key != key) break;


					if (index.hash != hash) continue;

					uint32_t neighborIndex = index.index;
					if (neighborIndex == particleIndex) continue;

					glm::vec2 neighbourPos = predictedPositions[neighborIndex];
					glm::vec2 offsetToNeighbour = (neighbourPos)-(pos);
					float sqrDist = dot(offsetToNeighbour, offsetToNeighbour);

					if (sqrDist > sqrRadius) continue;

					float dist = sqrt(sqrDist);
					glm::vec2 dir = dist > 0 ? offsetToNeighbour / dist : glm::vec2(0, 1);

					float neighborDensity = densities[neighborIndex].x;
					float neighborNearDensity = densities[neighborIndex].y;
					float neighborPressure = ConvertDensityToPressure(neighborDensity);
					float neighborNearPressure = ConvertNearDensityToPressure(neighborNearDensity);

					float sharedPressure = (pressure + neighborPressure) * 0.5f;
					float sharedNearPressure = (nearPressure + neighborNearPressure) * 0.5f;

					pressureForce += dir * kernels::SmoothingDerivativePow2(dist, interactionRadius) * sharedPressure / neighborDensity;
					pressureForce += dir * kernels::SmoothingDerivativePow3(dist, interactionRadius) * sharedNearPressure / neighborNearDensity;
				}
			}

			glm::vec2 acceleration = pressureForce / density;
			velocity[particleIndex] += acceleration * deltatime;
		}

		void FluidSimulation::CalculateViscosityForce(uint32 particleIndex, float deltatime)
		{
			glm::vec2 pos = predictedPositions[particleIndex];
			glm::vec2 originCell = PositionToCellCoord(pos);
			float sqrRadius = interactionRadius * interactionRadius;

			glm::vec2 viscosityForce = { 0,0 };
			glm::vec2 velo = velocity[particleIndex];

			for (int i = 0; i < 9; i++)
			{
				uint32_t hash = HashCell(originCell.x + offsets[i].x, originCell.y + offsets[i].y);
				uint32_t key = GetKeyFromHash(hash, numParticles);
				int currIndex = startIndices[key];

				while (currIndex < numParticles)
				{
					SpatialStruct index = spatialLookup[currIndex];
					currIndex++;
					if (index.key != key) break;


					if (index.hash != hash) continue;

					uint32_t neighborIndex = index.index;
					if (neighborIndex == particleIndex) continue;

					glm::vec2 neighbourPos = predictedPositions[neighborIndex];
					glm::vec2 offsetToNeighbour = neighbourPos-pos;
					float sqrDist = dot(offsetToNeighbour, offsetToNeighbour);

					if (sqrDist > sqrRadius) continue;

					float dist = sqrt(sqrDist);
					float influence = kernels::SmoothingViscoPoly6(dist, interactionRadius);
					viscosityForce += (velocity[neighborIndex] - velo) * influence;
				}
			}
			velocity[particleIndex] += viscosityForce * viscosityStrength * deltatime;
		}

		void FluidSimulation::UpdateSpatialLookup()
		{
			if (spatialLookup.size() == 0 || spatialLookup.size() < numParticles)
			{
				spatialLookup.resize(numParticles);
				startIndices.resize(numParticles);
			}
			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this](uint32_t i)
			{
				if (i >= numParticles) return;
				glm::vec2 cellPos = PositionToCellCoord(predictedPositions[i]);
				uint32_t hash = HashCell(cellPos.x, cellPos.y);
				uint32_t cellKey = GetKeyFromHash(hash, numParticles);
				spatialLookup[i] = { cellKey, hash, i };
				startIndices[i] = INT_MAX;
			});

			std::sort(spatialLookup.begin(), spatialLookup.end(), compareByKey);

			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this](uint32_t i)
			{
				if (i >= numParticles) return;
				uint32_t key = spatialLookup[i].key;
				uint32_t keyPrev = i == 0 ? UINT32_MAX : spatialLookup[i - 1].key;
				if (key != keyPrev)
				{
					startIndices[key] = i;
				}
			});

		}
		glm::vec2 FluidSimulation::PositionToCellCoord(const glm::vec2& pos)
		{
			glm::vec2 cell = floor(pos / interactionRadius);
			return { (int)cell.x, (int)cell.y };
		}

		uint32_t FluidSimulation::HashCell(int X, int Y)
		{
			uint32_t a = (uint32_t)X * 15823;
			uint32_t b = (uint32_t)Y * 9737333;
			return a + b;
		}

		uint32_t FluidSimulation::GetKeyFromHash(const uint32_t hash, const uint32_t spatialLength)
		{
			return hash % spatialLength;
		}

		void FluidSimulation::GridArrangement(int rowSize, float gap)
		{
			float bestGridColumAmount = 0;
			float bestGridRowAmount = 0;
			while (true)
			{
				float totalWidth = numParticles <= rowSize ? 0 : rowSize;

				float totalHeight = ceil((float)numParticles / (float)rowSize);

				float width = totalWidth * gap;
				float heigth = totalHeight * gap;

				if (width <= Bound.x && heigth <= Bound.y)
				{
					bestGridColumAmount = totalWidth;
					bestGridRowAmount = totalHeight;
					break;
				}
				rowSize++;
			}

			float TotalOffsetFromCenterWidth = bestGridColumAmount == 0 ? rowSize * gap : bestGridColumAmount * gap;
			float TotalOffsetFromCenterHeight = bestGridRowAmount * gap;

			for (int i = 0; i < numParticles; i++)
			{
				int localX = i % rowSize;
				int localY = i / rowSize;

				float XOffset = localX * gap;
				float YOffset = localY * gap;

				float worldOffsetX = (0 - ((TotalOffsetFromCenterWidth - gap) / 2.0f));
				float worldOffsetY = ( 0 + (TotalOffsetFromCenterHeight - gap) / 2.0f);
				float x = (worldOffsetX + XOffset);
				float y = (worldOffsetY - YOffset);
				positions[i] = { x,y };
				predictedPositions[i] = { x,y };
			}
		}
		void FluidSimulation::RandomArrangement(int gap)
		{
			auto arrangeTimeStart = std::chrono::steady_clock::now();
			int processed = 0;
			float sqrRadius = interactionRadius * interactionRadius;
			for (int i = 0; i < numParticles; i++)
			{
				if (processed == 0)
				{
					UpdateSpatialLookup();
				}

				if ((i % 10) == 10) UpdateSpatialLookup();

				// -1 .. 0 .. 1
				printf("Processing particle %i \n", i);
				float randomValueX = Core::RandomFloatNTP();
				float randomValueY = Core::RandomFloatNTP();

				bool PositionFail = false;

				float x = Bound.x * randomValueX;
				float y = Bound.y * randomValueY;
				glm::vec2 newPosition = { x,y };
				glm::vec2 originCell = PositionToCellCoord(newPosition);
				//printf("	New X %f \n", x);
				//printf("	New Y %f \n", y);

				for (int j = 0; j < 9; j++)
				{
					uint32_t hash = HashCell(originCell.x + offsets[j].x, originCell.y + offsets[j].y);
					uint32_t key = GetKeyFromHash(hash, numParticles);
					int currIndex = startIndices[key];

					while (currIndex < numParticles)
					{
						SpatialStruct index = spatialLookup[currIndex];
						currIndex++;
						if (index.key != key) break;


						if (index.hash != hash) continue;

						uint32_t neighborIndex = index.index;
						if (neighborIndex == i) continue;

						glm::vec2 neighbourPos = predictedPositions[neighborIndex];
						glm::vec2 offsetToNeighbour = neighbourPos - newPosition;
						float sqrDist = dot(offsetToNeighbour, offsetToNeighbour);

						if (sqrDist > sqrRadius) continue;

						PositionFail = true;
						break;
					}
				}

				if (PositionFail) { i--; continue; }

				positions[i] = newPosition;
				predictedPositions[i] = newPosition;
				processed++;
			}

			auto arrangeTimeEnd = std::chrono::steady_clock::now();
			double ArrangeTimeElapsed = std::chrono::duration<double>(arrangeTimeEnd - arrangeTimeStart).count() * 1000.0f; // ms
			printf("Time to arrange: %f ms\n", (float)ArrangeTimeElapsed);
		}
	}
}