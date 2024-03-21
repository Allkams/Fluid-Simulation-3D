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
#include "gameApp.h"

#include "imgui.h"

#include <vector>
#include <chrono>
#include <thread>
#include <execution>

#include "render/RenderBasic.h"
#include "render/shader.h"
#include "render/computeshader.h"
#include "render/camera.h"
#include "physics/physicsWorld.h"

#define particleAmount 25000


namespace Game
{

	GameApp::GameApp()
	{
		//Empty
	}

	GameApp::~GameApp()
	{
		//Empty
	}

	bool GameApp::Open()
	{
		this->window = new DISPLAY::Window;

		if (window->Open())
		{
			//this->window->setSize(1900, 1060);
			this->window->setSize(2304, 1296);
			this->window->setTitle("Fluid Sim");
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);


			this->window->SetUiRender([this]()
			{
				this->RenderUI();
			});

			return true;
		}

		return false;
	}

	bool GameApp::Run()
	{
		//NOTE: DOES NOT WORK AS INTENDED ON < 45 fps as computation takes to much time and gap for force and viscosity becomes uncontrollably high.

		/*TODO LIST
		*  - Add comments to code!
		*  - Implement a GPU Sort somehow...
		*  - Make the 3D Bound rotatable when pressing shift
		*  - Make camera movement when pressing ctrl
		*/

		glm::vec2 winSize = window->getSize();
		RenderUtils::Camera Cam(glm::vec3(0));
    
		nrParticles = particleAmount;
		Physics::Fluid::FluidSimulation::getInstance().InitializeData(nrParticles);
		std::vector<uint32_t> particles;
		for (int i = 0; i < nrParticles; i++)
		{
			particles.push_back(i);
		}

		Shader shader = Shader("./shaders/VertexShader.vs", "./shaders/FragementShader.fs");
		Shader particleShader = Shader("./shaders/particleRender.vs", "./shaders/particleRender.fs");
		Render::ComputeShader cParticleShader = Render::ComputeShader("./shaders/compute_particle.glsl");
		
		shader.Enable();

		Cam.setProjection(winSize.x, winSize.y, 0.01f, 1000.0f);
		Cam.setViewMatrix();


		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::translate(glm::vec3(0.0f, 0.0f, -3.0f));


		shader.setMat4("model", trans);
		shader.setMat4("view", Cam.GetViewMatrix());
		shader.setMat4("project", Cam.GetProjection());
		Render::Mesh Bound = Render::CreateCube(1.0f, 1.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		std::vector<glm::vec4> colors;
		colors.resize(nrParticles);

		std::for_each(std::execution::par, particles.begin(), particles.end(),
			[this, &colors](uint32_t i)
		{
			colors[i] = Color1;
		});

		std::vector<glm::mat4> transforms;
		transforms.resize(nrParticles);

		GLuint bufPositions;
		GLuint bufColors;
		GLuint bufPredictedPos;
		GLuint bufVelocities;
		GLuint bufDensities;
		GLuint bufSpatialIndices;
		GLuint bufSpatialOffsets;

		glGenBuffers(1, &bufPositions);
		glGenBuffers(1, &bufColors);
		glGenBuffers(1, &bufPredictedPos);
		glGenBuffers(1, &bufVelocities);
		glGenBuffers(1, &bufDensities);
		glGenBuffers(1, &bufSpatialIndices);
		glGenBuffers(1, &bufSpatialOffsets);

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

		cParticleShader.use();
		cParticleShader.setInt("NumParticles", nrParticles);

		cParticleShader.setFloat("interactionRadius", Physics::Fluid::FluidSimulation::getInstance().getInteractionRadius());
		cParticleShader.setFloat("targetDensity", Physics::Fluid::FluidSimulation::getInstance().getDensityTarget());
		cParticleShader.setFloat("pressureMultiplier", Physics::Fluid::FluidSimulation::getInstance().getPressureMultiplier());
		cParticleShader.setFloat("nearPressureMultiplier", Physics::Fluid::FluidSimulation::getInstance().getNearPressureMultiplier());
		cParticleShader.setFloat("viscosityStrength", Physics::Fluid::FluidSimulation::getInstance().getViscosityStrength());
		cParticleShader.setFloat("gravityScale", Physics::Fluid::FluidSimulation::getInstance().getGravityScale());

		cParticleShader.setVec3("boundSize", Physics::Fluid::FluidSimulation::getInstance().getBounds());
		cParticleShader.setVec3("centre", glm::vec3(0));
		glUseProgram(0);
		shader.Enable();

		//Cam.Position = { 20, 0, 0 };
		Cam.Position = { 10, 15, 10 };
		Cam.Target = { 0,0,0 };
		Cam.setViewMatrix(true);
		Cam.shouldTarget = true;
		Cam.setViewProjection();
		shader.setMat4("view", Cam.GetViewMatrix());

		deltatime = 0.016667f;
		while (this->window->IsOpen())
		{
			auto timeStart = std::chrono::steady_clock::now();
			//glClearColor(0.4f, 0.0f, 0.8f, 1.0f);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			colors.resize(nrParticles);
			transforms.resize(nrParticles);

			// Accept fragment if it closer to the camera than the former one
			glDepthFunc(GL_LESS);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (this->window->ProcessInput(GLFW_KEY_F1))
			{
				shader.ReloadShader();
				shader.Enable();
				shader.setMat4("transform", Cam.GetViewMatrix());
			}

			if (this->window->ProcessInput(GLFW_KEY_ESCAPE))
			{
				this->window->Close();
				break;
			}

			if (this->window->ProcessInput(GLFW_KEY_W))
			{
				Cam.Move(RenderUtils::FORWARD, deltatime);
			}
			if (this->window->ProcessInput(GLFW_KEY_S))
			{
				Cam.Move(RenderUtils::BACKWARD, deltatime);
			}
			if (this->window->ProcessInput(GLFW_KEY_A))
			{
				Cam.Move(RenderUtils::LEFT, deltatime);
			}
			if (this->window->ProcessInput(GLFW_KEY_D))
			{
				Cam.Move(RenderUtils::RIGHT, deltatime);
			}
			if (this->window->ProcessInput(GLFW_KEY_Q))
			{
				Cam.Move(RenderUtils::UP, deltatime);
			}
			if (this->window->ProcessInput(GLFW_KEY_E))
			{
				Cam.Move(RenderUtils::DOWN, deltatime);
			}
			Cam.setViewMatrix(true);
			shader.setMat4("view", Cam.GetViewMatrix());

			if (this->window->ProcessInput(GLFW_KEY_SPACE) && isRunning)
			{
				isRunning = false;
			}
			else if (this->window->ProcessInput(GLFW_KEY_SPACE) && !isRunning)
			{
				isRunning = true;
			}


			auto simStart = std::chrono::steady_clock::now();
			if (isRunning)
			{
				// RUN UPDATE
				Physics::Fluid::FluidSimulation::getInstance().Update(deltatime);

			}
			auto simEnd = std::chrono::steady_clock::now();
			double simElapsed = std::chrono::duration<double>(simEnd - simStart).count() * 1000.0f;
			Physics::Fluid::FluidSimulation::getInstance().setSimulationTime((float)simElapsed);

			auto colorUpdateStart = std::chrono::steady_clock::now();
			// --------------------------------------------------------------------------------------------
			// Update Colors and Transition
			// --------------------------------------------------------------------------------------------
			std::for_each(std::execution::par, particles.begin(), particles.end(),
				[this, &colors, &transforms](uint32_t i)
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
			// --------------------------------------------------------------------------------------------
			auto colorUpdateEnd = std::chrono::steady_clock::now();
			colorElapsed = std::chrono::duration<double>(colorUpdateEnd - colorUpdateStart).count() * 1000.0f;

			auto renderStart = std::chrono::steady_clock::now();

			shader.Disable();

			//Compute shader does not work...

			/*if (isRunning)
			{
				cParticleShader.use();
				cParticleShader.setFloat("TimeStep", deltatime);

				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufPositions);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufColors);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufPredictedPos);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bufVelocities);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, bufDensities);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, bufSpatialIndices);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, bufSpatialOffsets);

				const int numWorkGroups[3] = {
					nrParticles / 1024,
					1,
					1
				};

				glDispatchCompute(numWorkGroups[0], numWorkGroups[1], numWorkGroups[2]);
			}*/

			// Particle Drawing
			particleShader.Enable();

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufPositions);
			glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(glm::vec4), &Physics::Fluid::FluidSimulation::getInstance().OutPositions[0], GL_DYNAMIC_DRAW);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufColors);
			glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(glm::vec4), &colors[0], GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glm::mat4 billboardView = glm::mat4(
				{ 1, 0, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 1, 0 },
				Cam.GetViewMatrix()[3]
			);
			glm::mat4 billboardViewProjection = Cam.GetProjection() * billboardView;
			Cam.setViewProjection();
			particleShader.setMat4("ViewProj", Cam.GetViewProjection());
			particleShader.setMat4("BillBoardViewProj", billboardViewProjection);
			GLuint particleOffsetLoc = glGetUniformLocation(particleShader.GetProgram(), "ParticleOffset");

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

			particleShader.Disable();

			shader.Enable();

			shader.setMat4("view", Cam.GetViewMatrix());
			shader.setMat4("project", Cam.GetProjection());

			//BOUND rendering
			glm::vec3 boundScale = Physics::Fluid::FluidSimulation::getInstance().getBounds();
			shader.setVec4("color", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
			trans = glm::translate(glm::vec3(0.0f,0.0f, 0.0f)) * glm::scale(boundScale);
			shader.setMat4("model", trans);
			//Make a real bound instead of just a wireframe.
			glPolygonMode(GL_FRONT, GL_LINE);
			glPolygonMode(GL_BACK, GL_LINE);
			Bound.bindVAO();
			Bound.renderMesh(0);
			Bound.unBindVAO();
			glPolygonMode(GL_FRONT, GL_FILL);
			glPolygonMode(GL_BACK, GL_FILL);

			Cam.setViewProjection();
			auto renderEnd = std::chrono::steady_clock::now();
			renderingElapsed = std::chrono::duration<double>(renderEnd - renderStart).count() * 1000.0f;
			this->window->SwapBuffers();
			this->window->Update();

			auto timeEnd = std::chrono::steady_clock::now();
			deltatime = std::min((1.0/2.0), std::chrono::duration<double>(timeEnd - timeStart).count());
		}

		glDeleteBuffers(1, &bufPositions);
		glDeleteBuffers(1, &bufColors);
		glDeleteBuffers(1, &bufPredictedPos);
		glDeleteBuffers(1, &bufVelocities);
		glDeleteBuffers(1, &bufDensities);
		glDeleteBuffers(1, &bufSpatialIndices);
		glDeleteBuffers(1, &bufSpatialOffsets);

		return true;
	}

	bool GameApp::Close()
	{
		this->window->Close();
		return true;
	}

	void GameApp::RenderUI()
	{
		if (this->window->IsOpen())
		{
			ImGui::Begin("Debug Info");

			int fps = 1.0f/ deltatime;
			ImGui::Text("FPS: %i", fps);
			ImGui::Text("Number of Particles: %i", nrParticles);
			ImGui::NewLine();
			if(ImGui::CollapsingHeader("SIMULATION DATA"))
			{
				ImGui::Text("Simulation status: %s", isRunning ? "ON" : "OFF");
				float simTime = Physics::Fluid::FluidSimulation::getInstance().getSimulationTime();
				ImGui::Text("Simulation Elapsed: %.2f ms", simTime);
				ImGui::Text("Rendering Elapsed:  %.2f ms", renderingElapsed);
				ImGui::Text("Color Elapsed:      %.2f ms", colorElapsed);
				ImGui::Text("Program Elapsed:    %.2f ms", deltatime * 1000.0f);
			}
			if (ImGui::CollapsingHeader("PARTICLE DATA"))
			{
				int targetParticle = CurrentParticle;
				if (ImGui::InputInt("Focus Particle", &targetParticle))
				{
					if (targetParticle >= nrParticles)
					{
						targetParticle = 0;
					}
					if (targetParticle <= -1)
					{
						targetParticle = nrParticles - 1;
					}
					CurrentParticle = targetParticle;
				}
				glm::vec3 pos = Physics::Fluid::FluidSimulation::getInstance().getPosition(CurrentParticle);
				glm::vec3 vel = Physics::Fluid::FluidSimulation::getInstance().getVelocity(CurrentParticle);
				ImGui::Text("  Position: (%f, %f, %f)", pos.x, pos.y, pos.z);
				ImGui::Text("  Velocity: (%f, %f, %f)", vel.x, vel.y, vel.z);
				ImGui::Text("  Density: (%f)", Physics::Fluid::FluidSimulation::getInstance().getDensity(CurrentParticle));
				ImGui::Text("  Near Density: (%f)", Physics::Fluid::FluidSimulation::getInstance().getNearDensity(CurrentParticle));
			}

			ImGui::End();

			ImGui::Begin("Values");

			if (isRunning)
			{
				isRunning = !ImGui::Button("Stop", { 100,25 });
			}
			else
			{
				isRunning = ImGui::Button("Start", { 100,25 });
				ImGui::SameLine();
				if (ImGui::Button("Reset", { 100,25 }))
				{
					deltatime = 0.0166667f;
					Physics::Fluid::FluidSimulation::getInstance().InitializeData(nrParticles);
				}
			}

			bool gravity = Physics::Fluid::FluidSimulation::getInstance().getGravityStatus();
			if (ImGui::Checkbox("Gravity", &gravity))
			{
				Physics::Fluid::FluidSimulation::getInstance().setGravity(gravity);
			}

			float interactionRadius = Physics::Fluid::FluidSimulation::getInstance().getInteractionRadius();
			if (ImGui::SliderFloat("Interaction Radius", &interactionRadius, 0.01f, 10.0f))
			{
				Physics::Fluid::FluidSimulation::getInstance().setInteractionRadius(interactionRadius);
			}

			float TargetDensity = Physics::Fluid::FluidSimulation::getInstance().getDensityTarget();
			if (ImGui::SliderFloat("Target Density", &TargetDensity, 0.0f, 100.0f))
			{
				Physics::Fluid::FluidSimulation::getInstance().setDensityTarget(TargetDensity);
			}

			float pressureMulti = Physics::Fluid::FluidSimulation::getInstance().getPressureMultiplier();
			if (ImGui::SliderFloat("Pressure Multiplier", &pressureMulti, 0.0f, 500.0f))
			{
				Physics::Fluid::FluidSimulation::getInstance().setPressureMultiplier(pressureMulti);
			}

			float nearPressureMulti = Physics::Fluid::FluidSimulation::getInstance().getNearPressureMultiplier();
			if (ImGui::SliderFloat("Pressure Near Multiplier", &nearPressureMulti, 0.0f, 100.0f))
			{
				Physics::Fluid::FluidSimulation::getInstance().setNearPressureMultiplier(nearPressureMulti);
			}

			float viscosityStrength = Physics::Fluid::FluidSimulation::getInstance().getViscosityStrength();
			if (ImGui::SliderFloat("Viscosity Strength", &viscosityStrength, 0.0f, 1.0f))
			{
				Physics::Fluid::FluidSimulation::getInstance().setViscosityStrength(viscosityStrength);
			}

			float gravityScale = Physics::Fluid::FluidSimulation::getInstance().getGravityScale();
			if (ImGui::SliderFloat("Gravity Scale", &gravityScale, 0.0f, 10.0f))
			{
				Physics::Fluid::FluidSimulation::getInstance().setGravityScale(gravityScale);
			}

			glm::vec3 bound = Physics::Fluid::FluidSimulation::getInstance().getBounds();
			float b[3] = {bound.x, bound.y, bound.z};
			if (ImGui::SliderFloat3("Bounding Volume", b, 0.0f, 30.0f))
			{
				Physics::Fluid::FluidSimulation::getInstance().setBound({b[0], b[1], b[2]});
			}

			if (ImGui::CollapsingHeader("COLORS"))
			{
				if (ImGui::CollapsingHeader("Color 1"))
				{
					ImGui::ColorPicker3("Color1", &Color1[0]);
				}
				if (ImGui::CollapsingHeader("Color 2"))
				{
					ImGui::ColorPicker3("Color2", &Color2[0]);
				}
				if (ImGui::CollapsingHeader("Color 3"))
				{
					ImGui::ColorPicker3("Color3", &Color3[0]);
				}
				if (ImGui::CollapsingHeader("Color 4"))
				{
					ImGui::ColorPicker3("Color4", &Color4[0]);
				}
			}

			ImGui::End();
		}
	}
}