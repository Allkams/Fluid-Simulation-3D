#include "config.h"
#include "physicsWorld.h"

#include "kernels.h"

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
					velocity[i].y += 9.82f * deltatime;
				}
				predictedPositions[i] = positions[i] + velocity[i] * (1.0f / 60.0f);
			});

			UpdateSpatialLookup();

			updateDensities();

			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this, deltatime](uint32_t i)
			{
				glm::vec2 pressureForce = CalculatePressureForce(i);
				glm::vec2 pressureAcceration = pressureForce / densities[i];
				velocity[i] += pressureAcceration * deltatime;
			});

			//std::for_each(std::execution::par, pList.begin(), pList.end(),
			//	[this, deltatime](uint32_t i)
			//{
			//	glm::vec2 viscosityForce = CalculateViscosityForce(i);
			//	glm::vec2 viscosityAcceration = viscosityForce / densities[i];
			//	velocity[i] += viscosityAcceration * deltatime;
			//});

			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this, deltatime](uint32_t i)
			{
				const float dampFactor = 0.90f;
				positions[i] += deltatime * velocity[i];
				//printf("Particle %i Positions: %f, %f \n", i, positions[i].x, positions[i].y);
				//printf("Particle %i Velocity: %f, %f \n", i, velocity[i].x, velocity[i].y);
				//velocity[i] *= 0.99f;

				const glm::vec2 halfSize = glm::vec2(12,8) * 0.5f;
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

			for (int i = 0; i < particleAmmount; i++)
			{
				pList.push_back(i);
			}

			for (int i = 0; i < particleAmmount; i++)
			{
				positions.push_back({ 0,0 });
				velocity.push_back({ 0,0 });
				predictedPositions.push_back({ 0,0 });
				densities.push_back(0);
			}

			int RowSize = glm::sqrt(particleAmmount);
			float gap = 0.2f;
			switch (type)
			{
			case Physics::Fluid::GRID:
				GridArrangement(RowSize, gap);
				break;
			case Physics::Fluid::RANDOM:
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

		glm::vec2& FluidSimulation::getPosition(uint32 particleIndex)
		{
			return positions[particleIndex];
		}

		glm::vec2& FluidSimulation::getVelocity(uint32 particleIndex)
		{
			return velocity[particleIndex];
		}

		float FluidSimulation::getDensity(uint32 particleIndex)
		{
			return densities[particleIndex];
		}

		float FluidSimulation::getSpeed(uint32 particleIndex)
		{
			return glm::length(velocity[particleIndex]);
		}

		float FluidSimulation::getSpeedNormalzied(uint32 particleIndex)
		{
			return glm::clamp(glm::length(velocity[particleIndex]), 0.0f, 1.0f) / 1.0f;
		}

		void FluidSimulation::updateDensities()
		{
			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this](uint32_t i)
			{
				densities[i] = CalculateDensity(i);
			});
		}

		float FluidSimulation::CalculateDensity(uint32 particleIndex)
		{
			float density = 0;

			glm::vec2 pressureForce = { 0,0 };
			glm::vec2 pos = predictedPositions[particleIndex];
			glm::vec2 originCell = PositionToCellCoord(pos);
			float sqrRadius = interactionRadius * interactionRadius;

			for (int i = 0; i < 9; i++)
			{
				uint32_t hash = HashCell(originCell.x + offsets[i].x, originCell.y + offsets[i].y);
				uint32_t key = GetKeyFromHash(hash, spatialLookup.size());
				int currIndex = startIndices[key];

				while (currIndex < pList.size())
				{
					SpatialStruct index = spatialLookup[currIndex];
					currIndex++;
					if (index.key != key) break;


					if (index.hash != hash) continue;

					uint32_t neighborIndex = index.index;
					//if (neighborIndex == particleIndex) continue;

					glm::vec2 neighbourPos = predictedPositions[neighborIndex];
					glm::vec2 offsetToNeighbour = (neighbourPos)-(pos);
					float sqrDist = dot(offsetToNeighbour, offsetToNeighbour);

					if (sqrDist > sqrRadius) continue;

					float dist = sqrt(sqrDist);
					density += kernels::SmoothingPow2(dist, interactionRadius);
				}
			}

			return density;
		}

		float FluidSimulation::ConvertDensityToPressure(float density)
		{
			return (density - TargetDensity) * pressureMultiplier;
		}

		float FluidSimulation::CalculateSharedPressure(float densityA, float densityB)
		{
			float pressureA = ConvertDensityToPressure(densityA);
			float pressureB = ConvertDensityToPressure(densityB);
			return (pressureA + pressureB) * 0.5f;
		}

		glm::vec2& FluidSimulation::CalculatePressureForce(uint32 particleIndex)
		{
			glm::vec2 pressureForce = { 0,0 };
			glm::vec2 pos = predictedPositions[particleIndex];
			glm::vec2 originCell = PositionToCellCoord(pos);
			float sqrRadius = interactionRadius * interactionRadius;

			for (int i = 0; i < 9; i++)
			{
				uint32_t hash = HashCell(originCell.x + offsets[i].x, originCell.y + offsets[i].y);
				uint32_t key = GetKeyFromHash(hash, spatialLookup.size());
				int currIndex = startIndices[key];

				while (currIndex < pList.size())
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

					if (sqrDist <= sqrRadius)
					{
						//callback.push_back(index);
						float dist = glm::length(offsetToNeighbour);
						glm::vec2 dir = dist > 0.0f ? offsetToNeighbour / dist : glm::vec2(0, 1);
						float slope = kernels::SmoothingDerivativePow2(dist, interactionRadius);
						float density = densities[neighborIndex];
						float sharedPressure = CalculateSharedPressure(density, densities[particleIndex]);
						pressureForce += dir * slope * sharedPressure / density;
					}
				}
			}

			return pressureForce;
		}

		glm::vec2& FluidSimulation::CalculateViscosityForce(uint32 particleIndex)
		{
			glm::vec2 viscosityForce = { 0,0 };
			glm::vec2 pos = predictedPositions[particleIndex];
			glm::vec2 originCell = PositionToCellCoord(pos);
			float sqrRadius = interactionRadius * interactionRadius;

			for (int i = 0; i < 9; i++)
			{
				uint32_t hash = HashCell(originCell.x + offsets[i].x, originCell.y + offsets[i].y);
				uint32_t key = GetKeyFromHash(hash, spatialLookup.size());
				int currIndex = startIndices[key];

				while (currIndex < pList.size())
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

					if (sqrDist <= sqrRadius)
					{
						//callback.push_back(index);
						float dist = glm::length(offsetToNeighbour);
						float density = densities[neighborIndex];
						float influence = kernels::SmoothingViscoPoly6(dist, interactionRadius);
						viscosityForce += (velocity[neighborIndex] - velocity[particleIndex] * influence);
					}
				}
			}
			viscosityForce *= viscosityStrength;
			return viscosityForce;
		}

		void FluidSimulation::UpdateSpatialLookup()
		{
			if (spatialLookup.size() == 0)
			{
				spatialLookup.resize(numParticles);
				startIndices.resize(numParticles);
			}
			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this](uint32_t i)
			{
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
			float totalWidth = numParticles < rowSize ? 0 : numParticles % rowSize;
			float TotalOffsetFromCenterWidth = totalWidth == 0 ? rowSize * gap : totalWidth * gap;

			float totalHeight = ceil((float)numParticles / (float)rowSize);
			float TotalOffsetFromCenterHeight = totalHeight * gap;

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
	}
}