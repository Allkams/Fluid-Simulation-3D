#pragma once

#include "fluidSimBase.h"
#include "render/computeshader.h"
#include <vector>

class FluidSimGPU : public FluidSimBase
{
public:
	void initialize(int particleAmount) override;
	void update(float dt) override;
	void reset() override;
	void updateGPUBufferData();
	void cleanup() override;
	void render(Shader& renderShader,RenderUtils::Camera& cam) override;
	FluidSimGPU();

private:
	int nrParticles;
	int numWorkGroups[3] = {1,1,1};
	std::vector<bool> Particles;
	std::vector<glm::vec4> colors;

	GLuint bufPositions;
	GLuint bufColors;
	GLuint bufPredictedPos;
	GLuint bufVelocities;
	GLuint bufDensities;
	GLuint bufSpatialIndices;
	GLuint bufSpatialOffsets;

	Render::ComputeShader cParticleShader;
	Render::ComputeShader cPredictPositionShader;
	Render::ComputeShader cComputeDensityShader;
	Render::ComputeShader cComputePressureShader;
	Render::ComputeShader cComputeViscosityShader;
	Render::ComputeShader cUpdatePositionShader;

	Render::ComputeShader cSpatialHashShader;
	Render::ComputeShader cBitonicSortShader;
	Render::ComputeShader cSpatialOffsetsShader;

	//Will be moved to an singleton ish - Needs to be changable from multiple places.
	glm::vec4 Color1 = { 0.0f, 0.75f, 1.0f, 1.0f };
	glm::vec4 Color2 = { 0.0f, 1.0f, 0.0f, 1.0f };
	glm::vec4 Color3 = { 1.0f, 1.0f, 0.0f, 1.0f };
	glm::vec4 Color4 = { 1.0f, 0.0f, 0.0f, 1.0f };

private:
	int nextPowerOfTwo(uint n);

	void updateColors();

};