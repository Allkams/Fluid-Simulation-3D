#pragma once

#include "fluidSimBase.h"

class FluidSimCPU : public FluidSimBase
{
public:
	void initialize(int particleAmount) override;
	void update(float dt) override;
	void reset() override;
	void cleanup() override;
};