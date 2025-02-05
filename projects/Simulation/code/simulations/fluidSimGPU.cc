#include "config.h"
#include "fluidSimGPU.h"
#include <vector>
#include <chrono>
#include <thread>
#include <execution>
#include "physics/physicsWorld.h"

void FluidSimGPU::initialize(int particleAmount)
{
	// Load GPU shaders, buffers, etc.
	cParticleShader = Render::ComputeShader("./shaders/compute_particle.glsl");
	cPredictPositionShader = Render::ComputeShader("./compute/c_PredictPosition.glsl");
	cSpatialHashShader = Render::ComputeShader("./compute/c_SpatialHash.glsl");
	cBitonicSortShader = Render::ComputeShader("./compute/c_BitonicSort.glsl");
	cSpatialOffsetsShader = Render::ComputeShader("./compute/c_SpatialOffsets.glsl");
	cComputeDensityShader = Render::ComputeShader("./compute/c_ComputeDensity.glsl");
	//cComputePressureShader = Render::ComputeShader("./compute/c_ComputePressure.glsl");
	//cComputeViscosityShader = Render::ComputeShader("./compute/c_ComputeViscosity.glsl");
	//cUpdatePositionShader = Render::ComputeShader("./compute/c_UpdatePositions.glsl");
	
	//TempSolution
	nrParticles = particleAmount;
	Particles.resize(particleAmount);
	colors.resize(particleAmount); // Something happens here...

	//std::for_each(std::execution::par, Particles.begin(), Particles.end(),
	//	[this](uint32_t i)
	//{
	//	colors[i] = Color1;
	//});
	//-----

	numWorkGroups[0] = particleAmount / 1024;

	glGenBuffers(1, &bufPositions);
	glGenBuffers(1, &bufColors);
	glGenBuffers(1, &bufPredictedPos);
	glGenBuffers(1, &bufVelocities);
	glGenBuffers(1, &bufDensities);
	glGenBuffers(1, &bufSpatialIndices);
	glGenBuffers(1, &bufSpatialOffsets);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufPositions);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleAmount * sizeof(glm::vec4), &Physics::Fluid::FluidSimulation::getInstance().OutPositions[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufColors);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleAmount * sizeof(glm::vec4), &colors[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufPredictedPos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleAmount * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufVelocities);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleAmount * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufDensities);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleAmount * sizeof(glm::vec2), NULL, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufSpatialIndices);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleAmount * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufSpatialOffsets);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleAmount * sizeof(uint), NULL, GL_DYNAMIC_DRAW);

	cParticleShader.use();
	cParticleShader.setInt("NumParticles", particleAmount);

	cParticleShader.setFloat("interactionRadius", Physics::Fluid::FluidSimulation::getInstance().getInteractionRadius());
	cParticleShader.setFloat("targetDensity", Physics::Fluid::FluidSimulation::getInstance().getDensityTarget());
	cParticleShader.setFloat("pressureMultiplier", Physics::Fluid::FluidSimulation::getInstance().getPressureMultiplier());
	cParticleShader.setFloat("nearPressureMultiplier", Physics::Fluid::FluidSimulation::getInstance().getNearPressureMultiplier());
	cParticleShader.setFloat("viscosityStrength", Physics::Fluid::FluidSimulation::getInstance().getViscosityStrength());
	cParticleShader.setFloat("gravityScale", Physics::Fluid::FluidSimulation::getInstance().getGravityScale());

	cParticleShader.setVec3("boundSize", Physics::Fluid::FluidSimulation::getInstance().getBounds());
	cParticleShader.setVec3("centre", glm::vec3(0));

	//cParticleShader.setVec4("Color1", Color1);
	//cParticleShader.setVec4("Color2", Color2);
	//cParticleShader.setVec4("Color3", Color3);
	//cParticleShader.setVec4("Color4", Color4);

	cPredictPositionShader.use();
	cPredictPositionShader.setInt("NumParticles", particleAmount);
	cPredictPositionShader.setFloat("gravityScale", Physics::Fluid::FluidSimulation::getInstance().getGravityScale());

	cSpatialHashShader.use();
	cSpatialHashShader.setInt("NumParticles", particleAmount);
	cSpatialHashShader.setFloat("interactionRadius", Physics::Fluid::FluidSimulation::getInstance().getInteractionRadius());


	cBitonicSortShader.use();
	cBitonicSortShader.setInt("NumParticles", particleAmount);

	cSpatialOffsetsShader.use();
	cSpatialOffsetsShader.setInt("NumParticles", particleAmount);

	cComputeDensityShader.use();
	cComputeDensityShader.setInt("NumParticles", particleAmount);
	cComputeDensityShader.setFloat("interactionRadius", Physics::Fluid::FluidSimulation::getInstance().getInteractionRadius());
}

void FluidSimGPU::update(float dt)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufPositions);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufColors);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufPredictedPos);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bufVelocities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, bufDensities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, bufSpatialIndices);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, bufSpatialOffsets);

	cPredictPositionShader.use();
	cPredictPositionShader.setFloat("TimeStep", dt);
	glDispatchCompute(numWorkGroups[0], numWorkGroups[1], numWorkGroups[2]);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	cSpatialHashShader.use();
	glDispatchCompute(numWorkGroups[0], numWorkGroups[1], numWorkGroups[2]);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	cBitonicSortShader.use();
	int numStages = (int)log2f(nextPowerOfTwo(nrParticles));
	for (int stage = 1; stage <= numStages; stage++) {
		for (int passOfStage = 0; passOfStage < stage; passOfStage++) {
			// Update uniforms
			int groupWidth = 1 << (stage - passOfStage);
			int groupHeight = 2 * groupWidth - 1;

			cBitonicSortShader.setInt("groupWidth", groupWidth);
			cBitonicSortShader.setInt("groupHeight", groupHeight);
			cBitonicSortShader.setInt("StepIndex", stage);

			// Dispatch compute shader
			glDispatchCompute(nextPowerOfTwo(nrParticles) / 2, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}
	}

	cSpatialOffsetsShader.use();
	glDispatchCompute(numWorkGroups[0], numWorkGroups[1], numWorkGroups[2]);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	cComputeDensityShader.use();
	glDispatchCompute(numWorkGroups[0], numWorkGroups[1], numWorkGroups[2]);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	cParticleShader.use();
	cParticleShader.setFloat("TimeStep", dt);
	glDispatchCompute(numWorkGroups[0], numWorkGroups[1], numWorkGroups[2]);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufPositions);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufColors);

}

void FluidSimGPU::reset()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufPositions);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(glm::vec4), &Physics::Fluid::FluidSimulation::getInstance().OutPositions[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufColors);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(glm::vec4), &colors[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufPredictedPos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufVelocities);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufDensities);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(glm::vec2), NULL, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufSpatialIndices);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufSpatialOffsets);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(uint), NULL, GL_DYNAMIC_DRAW);
}

void FluidSimGPU::updateGPUBufferData()
{
	cParticleShader.use();
	cParticleShader.setFloat("interactionRadius", Physics::Fluid::FluidSimulation::getInstance().getInteractionRadius());
	cParticleShader.setFloat("targetDensity", Physics::Fluid::FluidSimulation::getInstance().getDensityTarget());
	cParticleShader.setFloat("pressureMultiplier", Physics::Fluid::FluidSimulation::getInstance().getPressureMultiplier());
	cParticleShader.setFloat("nearPressureMultiplier", Physics::Fluid::FluidSimulation::getInstance().getNearPressureMultiplier());
	cParticleShader.setFloat("viscosityStrength", Physics::Fluid::FluidSimulation::getInstance().getViscosityStrength());
	cParticleShader.setFloat("gravityScale", Physics::Fluid::FluidSimulation::getInstance().getGravityScale());
	cParticleShader.setVec3("boundSize", Physics::Fluid::FluidSimulation::getInstance().getBounds());
	//cParticleShader.setVec4("Color1", Color1);
	//cParticleShader.setVec4("Color2", Color2);
	//cParticleShader.setVec4("Color3", Color3);
	//cParticleShader.setVec4("Color4", Color4);

	cPredictPositionShader.use();
	cPredictPositionShader.setFloat("gravityScale", Physics::Fluid::FluidSimulation::getInstance().getGravityScale());
}

void FluidSimGPU::cleanup()
{
	// Free/Remove GPU Resources
	glDeleteBuffers(1, &bufPositions);
	glDeleteBuffers(1, &bufColors);
	glDeleteBuffers(1, &bufPredictedPos);
	glDeleteBuffers(1, &bufVelocities);
	glDeleteBuffers(1, &bufDensities);
	glDeleteBuffers(1, &bufSpatialIndices);
	glDeleteBuffers(1, &bufSpatialOffsets);
}

FluidSimGPU::FluidSimGPU()
{
	
}

int FluidSimGPU::nextPowerOfTwo(uint n)
{
	uint power = 1;
	while (power < n) {
		power <<= 1;
	}
	return int(power);
}

// --------------------------------------------------------------------------------------------
// Update Colors
// --------------------------------------------------------------------------------------------
void FluidSimGPU::updateColors()
{
	std::for_each(std::execution::par, Particles.begin(), Particles.end(),
		[this](uint32_t i)
		{
		if (colors.size() > nrParticles || i >= nrParticles) return;
		float normalized = Physics::Fluid::FluidSimulation::getInstance().getSpeedNormalzied(i);

		// Define the breakpoints for color transitions
		float breakpoint1 = 0.33f; // 33% of the gradient
		float breakpoint2 = 0.66f; // 66% of the gradient

		// Calculate the colors based on the normalized value
		if (normalized <= breakpoint1) {
			colors[i] = (1.0f - normalized / breakpoint1) * Color1 + (normalized / breakpoint1) * Color2;
		}
		else if (normalized <= breakpoint2) {
			colors[i] = (1.0f - (normalized - breakpoint1) / (breakpoint2 - breakpoint1)) * Color2 +
				((normalized - breakpoint1) / (breakpoint2 - breakpoint1)) * Color3;
		}
		else {
			colors[i] = (1.0f - (normalized - breakpoint2) / (1.0f - breakpoint2)) * Color3 +
				((normalized - breakpoint2) / (1.0f - breakpoint2)) * Color4;
		}
		});
}