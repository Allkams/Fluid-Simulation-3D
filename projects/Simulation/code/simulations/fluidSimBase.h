#pragma once
#include "render/shader.h"
#include "render/camera.h"

class FluidSimBase
{
public:
	virtual void update(float dt) = 0;
	virtual void initialize(int particleAmount) = 0;
	virtual void reset() = 0;
	virtual void cleanup() = 0;
	virtual void render(Shader& renderShader, RenderUtils::Camera& cam) = 0;
	virtual ~FluidSimBase() = default;
};