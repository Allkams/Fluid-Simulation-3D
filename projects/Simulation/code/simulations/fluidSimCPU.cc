#include "config.h"
#include "fluidSimCPU.h"
#include <vector>
#include <chrono>
#include <thread>
#include <execution>
#include "physics/physicsWorld.h"

void FluidSimCPU::initialize(int particleAmount)
{
	// Load CPU Resources
	nrParticles = particleAmount;
	Physics::Fluid::FluidSimulation::getInstance().InitializeData(particleAmount);
	Particles.resize(particleAmount);
	for (int i = 0; i < particleAmount; i++)
	{
		Particles[i] = i;
	}
	colors.resize(particleAmount);
	for (int i = 0; i < particleAmount; i++)
	{
		colors[i] = Color1;
	}

	glGenBuffers(1, &bufPositions);
	glGenBuffers(1, &bufColors);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufPositions);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleAmount * sizeof(glm::vec4), &Physics::Fluid::FluidSimulation::getInstance().OutPositions[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufColors);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleAmount * sizeof(glm::vec4), &colors[0], GL_DYNAMIC_DRAW);
}

void FluidSimCPU::update(float dt)
{
	// Run CPU Simulation
	Physics::Fluid::FluidSimulation::getInstance().Update(dt);
	updateColors();
}

void FluidSimCPU::reset()
{
	// Reset CPU Resources
	Physics::Fluid::FluidSimulation::getInstance().InitializeData(nrParticles);
}

void FluidSimCPU::cleanup()
{
	// Free/Remove CPU Resources
}

void FluidSimCPU::render(Shader& renderShader, RenderUtils::Camera& cam)
{
	renderShader.Enable();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufPositions);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(glm::vec4), &Physics::Fluid::FluidSimulation::getInstance().OutPositions[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufColors);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(glm::vec4), &colors[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glm::mat4 billboardView = glm::mat4(
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		cam.GetViewMatrix()[3]
	);
	glm::mat4 billboardViewProjection = cam.GetProjection() * billboardView;
	cam.setViewProjection();
	renderShader.setMat4("ViewProj", cam.GetViewProjection());
	renderShader.setMat4("BillBoardViewProj", billboardViewProjection);
	GLuint particleOffsetLoc = glGetUniformLocation(renderShader.GetProgram(), "ParticleOffset");

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufPositions);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufColors);

	int numVerts = nrParticles * 6;
	const int numVertsPerDrawCall = 0x44580; // has to be divisible with 6
	int numDrawCalls = (1024 / numVertsPerDrawCall) + 1;
	int particleOffset = 0;
	while (numVerts > 0)
	{
		int drawVertCount = glm::min(numVerts, numVertsPerDrawCall);
		glUniform1i(particleOffsetLoc, particleOffset);
		glDrawArrays(GL_TRIANGLES, 0, drawVertCount);
		numVerts -= drawVertCount;
		particleOffset += drawVertCount / 6;
	}

	renderShader.Disable();
}

FluidSimCPU::FluidSimCPU()
{

}

void FluidSimCPU::updateColors()
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