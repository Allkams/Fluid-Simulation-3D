#pragma once

#include "fluidSimBase.h"
#include "vector"


class FluidSimCPU : public FluidSimBase
{
public:
	void initialize(int particleAmount) override;
	void update(float dt) override;
	void reset() override;
	void cleanup() override;
	void render(Shader& renderShader, RenderUtils::Camera& cam) override;
	FluidSimCPU();

private:
	int nrParticles;
	std::vector<int> Particles;
	std::vector<glm::vec4> colors;

	GLuint bufPositions;
	GLuint bufColors;

	//Will be moved to an singleton ish - Needs to be changable from multiple places.
	glm::vec4 Color1 = { 0.0f, 0.75f, 1.0f, 1.0f };
	glm::vec4 Color2 = { 0.0f, 1.0f, 0.0f, 1.0f };
	glm::vec4 Color3 = { 1.0f, 1.0f, 0.0f, 1.0f };
	glm::vec4 Color4 = { 1.0f, 0.0f, 0.0f, 1.0f };

private:
	void updateColors();

};