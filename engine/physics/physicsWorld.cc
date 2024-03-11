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
				velocity[i] += CalculateExternalFoce(positions[i], velocity[i]) * deltatime;

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

			//std::for_each(std::execution::par, pList.begin(), pList.end(),
			//	[this, deltatime](uint32_t i)
			//{
			//	velocity2[i] = velocity[i];
			//});

			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this, deltatime](uint32_t i)
			{
				CalculateViscosityForce(i, deltatime);
			});

			//std::for_each(std::execution::par, pList.begin(), pList.end(),
			//	[this, deltatime](uint32_t i)
			//{
			//	velocity[i] = velocity2[i];
			//});

			std::for_each(std::execution::par, pList.begin(), pList.end(),
				[this, deltatime](uint32_t i)
			{
				positions[i] += velocity[i] * deltatime;
				
				/*glm::mat4 modelMatrix = glm::translate(positions[i]);

				glm::vec3 localPosition = glm::inverse(modelMatrix) * glm::vec4(positions[i], 1.0f);
				glm::vec3 localVelocity = glm::inverse(modelMatrix) * glm::vec4(velocity[i], 0.0f);*/

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
				/*positions[i] = modelMatrix * glm::vec4(positions[i], 1.0f);
				velocity[i] = modelMatrix * glm::vec4(velocity[i], 0.0f);*/

			});
		}
		void FluidSimulation::InitializeData(int particleAmmount, glm::vec3 Centre, Arrangements type)
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
				positions.push_back({ 0,0,0 });
				OutPositions.push_back({ 0,0,0, 0.25f });
				velocity.push_back({ 0,0,0 });
				velocity2.push_back({ 0,0,0 });
				predictedPositions.push_back({ 0,0,0 });
				densities.push_back({ 0,0 });
			}

			int RowSize = ceil(powf(particleAmmount, (1.0f / 3.0f)));
			//int RowSize = ceil(glm::sqrt(particleAmmount));
			float gap = 0.215f;

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

		//void FluidSimulation::setMousePosition(glm::vec3 pos)
		//{
		//	InteractionMousePoint = pos;
		//}

		//glm::vec3 FluidSimulation::getMousePosition()
		//{
		//	return InteractionMousePoint;
		//}

		//void FluidSimulation::setInputStrength(float value)
		//{
		//	InteractionInputStrength = value;
		//}

		//float FluidSimulation::getInputStrength()
		//{
		//	return InteractionInputStrength;
		//}

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

			/*if (InteractionInputStrength != 0)
			{
				glm::vec3 PointOffset = InteractionMousePoint - pos;
				float sqrDist = dot(PointOffset, PointOffset);
				float sqrRadius = InteractionInputRadius * InteractionInputRadius;
				if (sqrDist < sqrRadius)
				{
					float dst = sqrt(sqrDist);
					float edgeT = (dst / InteractionInputRadius);
					float centreT = 1 - edgeT;
					glm::vec3 dirToCentre = PointOffset / dst;

					float gravityWeight = 1 - (centreT * glm::clamp(InteractionInputStrength / 10, 0.0f, 1.0f));
					glm::vec3 accel = gravityAccel * gravityWeight + dirToCentre * centreT * InteractionInputStrength;
					accel -= vel * centreT;
					return accel;
				}
			}*/

			return gravityAccel;
		}

		glm::vec2 FluidSimulation::CalculateDensity(const glm::vec3& pos)
		{
			const glm::vec3& originCell = PositionToCellCoord(pos);
			//float sqrRadius = interactionRadius * interactionRadius;
			float density = 0;
			float NearDensity = 0;

			for (int i = 0; i < 27; i++)
			{
				uint32_t hash = HashCell(originCell.x + offsets[i].x, originCell.y + offsets[i].y, originCell.z + offsets[i].z);
				uint32_t key = GetKeyFromHash(hash, numParticles);
				uint32 currIndex = startIndices[key];

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
			const float density = densities[particleIndex].x;
			const float nearDensity = densities[particleIndex].y;
			const float pressure = ConvertDensityToPressure(density);
			const float nearPressure = ConvertNearDensityToPressure(nearDensity);
			glm::vec3 pressureForce = { 0,0, 0 };

			const glm::vec3& pos = predictedPositions[particleIndex];
			const glm::vec3& originCell = PositionToCellCoord(pos);
			//float sqrRadius = interactionRadius * interactionRadius;

			for (int i = 0; i < 27; i++)
			{
				uint32_t hash = HashCell(originCell.x + offsets[i].x, originCell.y + offsets[i].y, originCell.z + offsets[i].z);
				uint32_t key = GetKeyFromHash(hash, numParticles);
				int currIndex = startIndices[key];

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
					float neighborPressure = ConvertDensityToPressure(neighborDensity);
					float neighborNearPressure = ConvertNearDensityToPressure(neighborNearDensity);

					float sharedPressure = (pressure + neighborPressure) * 0.5f;
					float sharedNearPressure = (nearPressure + neighborNearPressure) * 0.5f;

					float dist = sqrt(sqrDist);
					glm::vec3 dir = dist > 0 ? offsetToNeighbour / dist : glm::vec3(0, 1, 0);

					pressureForce += dir * kernels::SmoothingDerivativePow2(dist, interactionRadius) * sharedPressure / neighborDensity;
					pressureForce += dir * kernels::SmoothingDerivativePow3(dist, interactionRadius) * sharedNearPressure / neighborNearDensity;
				}
			}

			glm::vec3 acceleration = pressureForce / density;
			velocity[particleIndex] += acceleration * deltatime;
		}

		void FluidSimulation::CalculateViscosityForce(uint32 particleIndex, float deltatime)
		{
			const glm::vec3& pos = predictedPositions[particleIndex];
			const glm::vec3& originCell = PositionToCellCoord(pos);
			//float sqrRadius = interactionRadius * interactionRadius;

			glm::vec3 viscosityForce = { 0,0,0 };
			const glm::vec3& velo = velocity[particleIndex];

			for (int i = 0; i < 27; i++)
			{
				uint32_t hash = HashCell(originCell.x + offsets[i].x, originCell.y + offsets[i].y, originCell.z + offsets[i].z);
				uint32_t key = GetKeyFromHash(hash, numParticles);
				int currIndex = startIndices[key];

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
				uint32_t hash = HashCell(cellPos.x, cellPos.y, cellPos.z);
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

		uint32_t FluidSimulation::HashCell(int X, int Y, int Z)
		{
			uint32_t a = (uint32_t)X * 15823;
			uint32_t b = (uint32_t)Y * 9737333;
			uint32_t c = (uint32_t)Z * 440817757;
			return a + b + c;
		}

		uint32_t FluidSimulation::GetKeyFromHash(const uint32_t hash, const uint32_t spatialLength)
		{
			return hash % spatialLength;
		}

		void FluidSimulation::GridArrangement(int particlesPerAxis, float gap, const glm::vec3& centre)
		{
			int i = 0;


			float bestGridColumAmount = 0;
			float bestGridRowAmount = 0;
			float bestGridDepthAmount = 0;
			/*while (true)
			{
				float totalWidth = numParticles <= particlesPerAxis ? 0 : particlesPerAxis;

				float totalHeight = ceil((float)numParticles / (float)particlesPerAxis);

				float width = totalWidth * gap;
				float heigth = totalHeight * gap;
				float Depth = width;

				if (width <= BoundScale.x && heigth <= BoundScale.y && Depth <= BoundScale.z)
				{
					bestGridColumAmount = totalWidth;
					bestGridRowAmount = totalHeight;
					bestGridDepthAmount = totalWidth;
					break;
				}
				particlesPerAxis++;
			}*/

			float TotalOffsetFromCenterWidth = bestGridColumAmount == 0 ? particlesPerAxis * gap : bestGridColumAmount * gap;
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
						/*positions[i] = { px * 10.0f, py * 10.0f, pz * 10.0f };
						predictedPositions[i] = { px * 10.0f, py * 10.0f, pz * 10.0f };
						OutPositions[i] = { px * 10.0f, py * 10.0f, pz * 10.0f, 0.34f };*/
						i++;
					}
				}
			}

			/*float bestGridColumAmount = 0;
			float bestGridRowAmount = 0;
			float bestGridDepthAmount = 0;
			while (true)
			{
				float totalWidth = numParticles <= particlesPerAxis ? 0 : particlesPerAxis;

				float totalHeight = ceil((float)numParticles / (float)particlesPerAxis);

				float width = totalWidth * gap;
				float heigth = totalHeight * gap;
				float Depth = width;

				if (width <= BoundScale.x && heigth <= BoundScale.y && Depth <= BoundScale.z)
				{
					bestGridColumAmount = totalWidth;
					bestGridRowAmount = totalHeight;
					bestGridDepthAmount = totalWidth;
					break;
				}
				particlesPerAxis++;
			}

			float TotalOffsetFromCenterWidth = bestGridColumAmount == 0 ? particlesPerAxis * gap : bestGridColumAmount * gap;
			float TotalOffsetFromCenterHeight = bestGridRowAmount * gap;
			float TotalOffsetFromCenterDepth = TotalOffsetFromCenterWidth;

			for (int i = 0; i < numParticles; i++)
			{
				int localX = i % particlesPerAxis;
				int localY = i / particlesPerAxis;
				int localZ = i / (particlesPerAxis * particlesPerAxis);

				float XOffset = localX * gap;
				float YOffset = localY * gap;
				float ZOffset = localZ * gap;

				float worldOffsetX = (0 - ((TotalOffsetFromCenterWidth - gap) / 2.0f));
				float worldOffsetY = (0 + (TotalOffsetFromCenterHeight - gap) / 2.0f);
				float worldOffsetZ = (0 + (TotalOffsetFromCenterDepth - gap) / 2.0f);
				float x = (worldOffsetX + XOffset);
				float y = (worldOffsetY - YOffset);
				float z = (worldOffsetZ - ZOffset);
				positions[i] = { x,y, z };
				predictedPositions[i] = { x,y, z };
				OutPositions[i] = { x,y,z, 0.34f };
			}*/
		}
		void FluidSimulation::RandomArrangement(int gap)
		{
			printf("Service unavalaible!");
			//	auto arrangeTimeStart = std::chrono::steady_clock::now();
			//	int processed = 0;
			//	float sqrRadius = interactionRadius * interactionRadius;
			//	for (int i = 0; i < numParticles; i++)
			//	{
			//		if (processed == 0)
			//		{
			//			UpdateSpatialLookup();
			//		}

			//		if ((i % 10) == 10) UpdateSpatialLookup();

			//		// -1 .. 0 .. 1
			//		printf("Processing particle %i \n", i);
			//		float randomValueX = Core::RandomFloatNTP();
			//		float randomValueY = Core::RandomFloatNTP();

			//		bool PositionFail = false;

			//		float x = Bound.x * randomValueX;
			//		float y = Bound.y * randomValueY;
			//		glm::vec2 newPosition = { x,y };
			//		glm::vec2 originCell = PositionToCellCoord(newPosition);
			//		//printf("	New X %f \n", x);
			//		//printf("	New Y %f \n", y);

			//		for (int j = 0; j < 9; j++)
			//		{
			//			uint32_t hash = HashCell(originCell.x + offsets[j].x, originCell.y + offsets[j].y);
			//			uint32_t key = GetKeyFromHash(hash, numParticles);
			//			int currIndex = startIndices[key];

			//			while (currIndex < numParticles)
			//			{
			//				SpatialStruct index = spatialLookup[currIndex];
			//				currIndex++;
			//				if (index.key != key) break;


			//				if (index.hash != hash) continue;

			//				uint32_t neighborIndex = index.index;
			//				if (neighborIndex == i) continue;

			//				glm::vec2 neighbourPos = predictedPositions[neighborIndex];
			//				glm::vec2 offsetToNeighbour = neighbourPos - newPosition;
			//				float sqrDist = dot(offsetToNeighbour, offsetToNeighbour);

			//				if (sqrDist > sqrRadius) continue;

			//				PositionFail = true;
			//				break;
			//			}
			//		}

			//		if (PositionFail) { i--; continue; }

			//		positions[i] = newPosition;
			//		predictedPositions[i] = newPosition;
			//		processed++;
			//	}

			//	auto arrangeTimeEnd = std::chrono::steady_clock::now();
			//	double ArrangeTimeElapsed = std::chrono::duration<double>(arrangeTimeEnd - arrangeTimeStart).count() * 1000.0f; // ms
			//	printf("Time to arrange: %f ms\n", (float)ArrangeTimeElapsed);
			//}
		}
	}
}