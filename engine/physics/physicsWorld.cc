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

#include "config.h"
#include "physicsWorld.h"

#include "kernels.h"
#include "core/random.h"

#include <chrono>
#include <thread>
#include <execution>

namespace Physics
{
	namespace Fluid
	{
		FluidSimulation& FluidSimulation::getInstance()
		{
			static FluidSimulation instance;

			return instance;
		}

		void FluidSimulation::Update(float deltatime)
		{
			auto GravityStart = std::chrono::steady_clock::now();
			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this, deltatime](uint32_t i)
			{
				velocity[i] += CalculateExternalFoce(positions[i], velocity[i]) * deltatime;

				predictedPositions[i] = positions[i] + velocity[i] * (1.0f / 120.0f);
			});
			auto GravityEnd = std::chrono::steady_clock::now();
			ElapsedTimeGravity = std::chrono::duration<double>(GravityEnd - GravityStart).count() * 1000.0f;

			auto SpatialStart = std::chrono::steady_clock::now();
			UpdateSpatialLookup();
			auto SpatialEnd = std::chrono::steady_clock::now();
			ElapsedTimeSpatial = std::chrono::duration<double>(SpatialEnd - SpatialStart).count() * 1000.0f;

			auto DensityStart = std::chrono::steady_clock::now();
			updateDensities();
			auto DensityEnd = std::chrono::steady_clock::now();
			ElapsedTimeDensity = std::chrono::duration<double>(DensityEnd - DensityStart).count() * 1000.0f;

			auto PressureStart = std::chrono::steady_clock::now();
			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this, deltatime](uint32_t i)
			{
				CalculatePressureForce(i, deltatime);
			});
			auto PressureEnd = std::chrono::steady_clock::now();
			ElapsedTimePressure = std::chrono::duration<double>(PressureEnd - PressureStart).count() * 1000.0f;

			auto ViscosityStart = std::chrono::steady_clock::now();
			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this, deltatime](uint32_t i)
			{
				CalculateViscosityForce(i, deltatime);
			});
			auto ViscosityEnd = std::chrono::steady_clock::now();
			ElapsedTimeViscosity = std::chrono::duration<double>(ViscosityEnd - ViscosityStart).count() * 1000.0f;

			auto PosNCollStart = std::chrono::steady_clock::now();
			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this, deltatime](uint32_t i)
			{
				positions[i] += velocity[i] * deltatime;
				
				// Edge collision check
				const float dampFactor = 0.95f;
				const glm::vec3 halfSize = BoundScale * 0.5f;
				glm::vec3 edgeDst = halfSize - abs(positions[i]);

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

				if (edgeDst.z <= 0)
				{
					positions[i].z = halfSize.z * glm::sign(positions[i].z);
					velocity[i].z *= -1 * dampFactor;
				}
				OutPositions[i] = glm::vec4(positions[i], 0.34f);
			});
			auto PosNCollEnd = std::chrono::steady_clock::now();
			ElapsedTimePositionNCollision = std::chrono::duration<double>(PosNCollEnd - PosNCollStart).count() * 1000.0f;
		}
		void FluidSimulation::InitializeData(int particleAmmount, glm::vec3 Centre)
		{
			numParticles = particleAmmount;

			pList.resize(particleAmmount);
			for (int i = 0; i < particleAmmount; i++)
			{
				pList[i] = i;
			}

			positions.resize(particleAmmount);
			OutPositions.resize(particleAmmount);
			velocity.resize(particleAmmount);
			velocity2.resize(particleAmmount);
			predictedPositions.resize(particleAmmount);
			densities.resize(particleAmmount);

			for (size_t i = 0; i < particleAmmount; i++)
			{
				positions[i] = glm::zero<glm::vec3>();
				OutPositions[i] = { 0,0,0, 0.25f };
				velocity[i] = glm::zero<glm::vec3>();
				velocity2[i] = glm::zero<glm::vec3>();
				predictedPositions[i] = glm::zero<glm::vec3>();
				densities[i] = glm::zero<glm::vec2>();
			}

			int RowSize = ceil(powf(particleAmmount, (1.0f / 3.0f)));
			float gap = 0.215f;

			GridArrangement(RowSize, gap);

			UpdateSpatialLookup();
			updateDensities();

		}

		glm::vec3 FluidSimulation::getPosition(uint32 particleIndex)
		{
			if (particleIndex >= numParticles) return glm::zero<glm::vec3>();
			return positions[particleIndex];
		}

		glm::vec3 FluidSimulation::getVelocity(uint32 particleIndex)
		{
			if (particleIndex >= numParticles) return glm::zero<glm::vec3>();
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

		double FluidSimulation::getElapsedTimeGravity()
		{
			return ElapsedTimeGravity;
		}

		double FluidSimulation::getElapsedTimeSpatial()
		{
			return ElapsedTimeSpatial;
		}

		double FluidSimulation::getElapsedTimeDensity()
		{
			return ElapsedTimeDensity;
		}

		double FluidSimulation::getElapsedTimePressure()
		{
			return ElapsedTimePressure;
		}

		double FluidSimulation::getElapsedTimeViscosity()
		{
			return ElapsedTimeViscosity;
		}

		double FluidSimulation::getElapsedTimePosNColl()
		{
			return ElapsedTimePositionNCollision;
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

		void FluidSimulation::setBound(const glm::vec3& value)
		{
			BoundScale = value;
		}

		glm::vec3 FluidSimulation::getBounds()
		{
			return BoundScale;
		}

		void FluidSimulation::updateDensities()
		{
			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this](uint32_t i)
			{
				densities[i] = CalculateDensity(predictedPositions[i]);
			});
		}

		glm::vec3 FluidSimulation::CalculateExternalFoce(const glm::vec3& pos, const glm::vec3& vel)
		{
			glm::vec3 gravityAccel = { 0,0, 0 };

			if (gravity)
			{
				gravityAccel = { 0, -gravityScale, 0 };
			}

			return gravityAccel;
		}

		glm::vec2 FluidSimulation::CalculateDensity(const glm::vec3& pos)
		{
			const glm::vec3& originCell = PositionToCellCoord(pos);
			float density = 0;
			float NearDensity = 0;

			for (int i = 0; i < 27; i++)
			{
				// Fetch neighbor cells
				uint32_t hash = HashCell(originCell + offsets[i]);
				uint32_t key = GetKeyFromHash(hash, numParticles);
				uint32 currIndex = startIndices[key];

				// Loop over neigbor particles in neighbor cell
				while (currIndex < numParticles)
				{
					const glm::vec3& index = spatialLookup[currIndex];
					currIndex++;
					if (index.z != key) break;


					if (index.y != hash) continue;

					if (index.x >= numParticles) break;


					uint32_t neighborIndex = index.x;

					const glm::vec3& neighbourPos = predictedPositions[neighborIndex];
					const glm::vec3& offsetToNeighbour = neighbourPos - pos;
					float sqrDist = dot(offsetToNeighbour, offsetToNeighbour);

					if (sqrDist > sqrRadius) continue;

					float dist = sqrt(sqrDist);
					density += kernels::SmoothingPow2(dist, interactionRadius);
					NearDensity += kernels::SmoothingPow3(dist, interactionRadius);
				}
			}
			return { density, NearDensity };
		}

		void FluidSimulation::CalculatePressureForce(uint32 particleIndex, float deltatime)
		{
			const float density = densities[particleIndex].x;
			const float nearDensity = densities[particleIndex].y;
			const float pressure = (density - TargetDensity) * pressureMultiplier;
			const float nearPressure = nearDensity * nearPressureMultiplier;
			glm::vec3 pressureForce = { 0,0, 0 };

			const glm::vec3& pos = predictedPositions[particleIndex];
			const glm::vec3& originCell = PositionToCellCoord(pos);

			for (int i = 0; i < 27; i++)
			{
				// Fetch neighbor cells
				uint32_t hash = HashCell(originCell + offsets[i]);
				uint32_t key = GetKeyFromHash(hash, numParticles);
				int currIndex = startIndices[key];

				// Loop over neigbor particles in neighbor cell
				while (currIndex < numParticles)
				{
					const glm::vec3& index = spatialLookup[currIndex];
					currIndex++;
					if (index.z != key) break;


					if (index.y != hash) continue;

					uint32_t neighborIndex = index.x;
					if (neighborIndex == particleIndex) continue;

					glm::vec3 neighbourPos = predictedPositions[neighborIndex];
					glm::vec3 offsetToNeighbour = neighbourPos-pos;
					float sqrDist = dot(offsetToNeighbour, offsetToNeighbour);

					if (sqrDist > sqrRadius) continue;


					float neighborDensity = densities[neighborIndex].x;
					float neighborNearDensity = densities[neighborIndex].y;
					float neighborPressure = (neighborDensity - TargetDensity) * pressureMultiplier;
					float neighborNearPressure = neighborNearDensity * nearPressureMultiplier;

					float sharedPressure = (pressure + neighborPressure) * 0.5f;
					float sharedNearPressure = (nearPressure + neighborNearPressure) * 0.5f;

					float dist = sqrt(sqrDist);
					glm::vec3 dir = dist > 0 ? offsetToNeighbour / dist : glm::vec3(0, 1, 0);

					pressureForce += dir * kernels::SmoothingDerivativePow2(dist, interactionRadius) * sharedPressure / neighborDensity;
					pressureForce += dir * kernels::SmoothingDerivativePow3(dist, interactionRadius) * sharedNearPressure / neighborNearDensity;
				}
			}

			velocity[particleIndex] += (pressureForce / density) * deltatime;
		}

		void FluidSimulation::CalculateViscosityForce(uint32 particleIndex, float deltatime)
		{
			const glm::vec3& pos = predictedPositions[particleIndex];
			const glm::vec3& originCell = PositionToCellCoord(pos);

			glm::vec3 viscosityForce = { 0,0,0 };
			const glm::vec3& velo = velocity[particleIndex];

			for (int i = 0; i < 27; i++)
			{
				// Fetch neighbor cells
				uint32_t hash = HashCell(originCell + offsets[i]);
				uint32_t key = GetKeyFromHash(hash, numParticles);
				int currIndex = startIndices[key];

				// Loop over neigbor particles in neighbor cell
				while (currIndex < numParticles)
				{
					const glm::vec3& index = spatialLookup[currIndex];
					currIndex++;
					if (index.z != key) break;


					if (index.y != hash) continue;

					uint32_t neighborIndex = index.x;
					if (neighborIndex == particleIndex) continue;

					const glm::vec3& neighbourPos = predictedPositions[neighborIndex];
					glm::vec3 offsetToNeighbour = neighbourPos - pos;
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
				glm::vec3 cellPos = PositionToCellCoord(predictedPositions[i]);
				uint32_t hash = HashCell(cellPos);
				uint32_t cellKey = GetKeyFromHash(hash, numParticles);
				spatialLookup[i] = { i, hash, cellKey };
				startIndices[i] = INT_MAX;
			});

			std::sort(spatialLookup.begin(), spatialLookup.end(), compareByKey);

			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this](uint32_t i)
			{
				if (i >= numParticles) return;
				uint32_t key = spatialLookup[i].z;
				uint32_t keyPrev = i == 0 ? UINT32_MAX : spatialLookup[i - 1].z;
				if (key != keyPrev)
				{
					startIndices[key] = i;
				}
			});

		}
		glm::vec3 FluidSimulation::PositionToCellCoord(const glm::vec3& pos)
		{
			glm::vec3 cell = floor(pos / interactionRadius);
			return { (int)cell.x, (int)cell.y, (int)cell.z };
		}

		uint32_t FluidSimulation::HashCell(const glm::vec3& inCell)
		{
			uint32_t a = (uint32_t)inCell.x * 15823;
			uint32_t b = (uint32_t)inCell.y * 9737333;
			uint32_t c = (uint32_t)inCell.z * 440817757;
			return a + b + c;
		}

		uint32_t FluidSimulation::GetKeyFromHash(const uint32_t hash, const uint32_t spatialLength)
		{
			return hash % spatialLength;
		}

		void FluidSimulation::GridArrangement(int particlesPerAxis, float gap, const glm::vec3& centre)
		{
			int i = 0;

			float TotalOffsetFromCenterWidth = particlesPerAxis * gap;
			float TotalOffsetFromCenterHeight = particlesPerAxis * gap;
			float TotalOffsetFromCenterDepth = TotalOffsetFromCenterWidth;

			for (int localY = 0; localY < particlesPerAxis; localY++)
			{
				for (int localX = 0; localX < particlesPerAxis; localX++)
				{
					for (int localZ = 0; localZ < particlesPerAxis; localZ++)
					{
						if (i >= numParticles)
						{
							return;
						}

						float XOffset = localX * gap;
						float YOffset = localY * gap;
						float ZOffset = localZ * gap;

						float worldOffsetX = (0 - ((TotalOffsetFromCenterWidth - gap) / 2.0f));
						float worldOffsetY = (0 + (TotalOffsetFromCenterWidth - gap) / 2.0f);
						float worldOffsetZ = (0 - (TotalOffsetFromCenterWidth - gap) / 2.0f);

						float x = (worldOffsetX + XOffset);
						float y = (worldOffsetY - YOffset);
						float z = (worldOffsetZ + ZOffset);

						positions[i] = { x,y, z };
						predictedPositions[i] = { x,y, z };
						OutPositions[i] = { x,y,z, 0.34f };
						i++;
					}
				}
			}

		}
	}
}