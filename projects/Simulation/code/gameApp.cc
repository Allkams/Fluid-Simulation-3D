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

		//window->SetKeyPressFunction([this](int32 key, int32 scancode, int32 action, int32 mods)
		//{
		//	if (key == GLFW_KEY_ESCAPE) {
		//		this->window->Close();
		//	}

		//	this->inputManager.HandleKeyPressEvent(key, scancode, action, mods);
		//});

		//window->SetMousePressFunction([this](int32 key, int32 action, int32 mods) {
		//	this->inputManager.HandleMousePressEvent(key, action, mods);
		//});

		//window->SetMouseMoveFunction([this](float64 x, float64 y) {
		//	this->inputManager.mouse.px = x;
		//	this->inputManager.mouse.py = y;
		//});

		/*this->window*/
		if (window->Open())
		{
			this->window->setSize(1900, 1060);
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
		*  - Implement a GPU Sort somehow...
		*  - Make the 3D Bound rotatable when pressing shift
		*  - Make camera movement when pressing ctrl
		*  - 
		*
		*/

		glm::vec2 winSize = window->getSize();
		//PreWork
		RenderUtils::Camera Cam(glm::vec3(0));
    
		nrParticles = 50000;
		Physics::Fluid::FluidSimulation::getInstace().InitializeData(nrParticles);
		std::vector<uint32_t> particles;
		for (int i = 0; i < nrParticles; i++)
		{
			particles.push_back(i);
		}

		Shader shader = Shader("./shaders/VertexShader.vs", "./shaders/FragementShader.fs");
		Shader particleShader = Shader("./shaders/particleRender.vs", "./shaders/particleRender.fs");
		//Render::ComputeShader cSpatial = Render::ComputeShader("./shaders/spatialHash.glsl");
		//Render::ComputeShader cMath = Render::ComputeShader("./shaders/FluidMath.glsl");
		Render::ComputeShader cParticleShader = Render::ComputeShader("./shaders/compute_particle.glsl");
		
		shader.Enable();

		Cam.setProjection(winSize.x, winSize.y, 0.01f, 1000.0f);
		Cam.setViewMatrix();


		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::translate(glm::vec3(0.0f, 0.0f, -3.0f));


		shader.setMat4("model", trans);
		shader.setMat4("view", Cam.GetViewMatrix());
		shader.setMat4("project", Cam.GetProjection());
		Render::Mesh plane2 = Render::CreateCircle(0.35f, 12);
		//Solve this problem...
		Render::Mesh plane3 = Render::CreateCube(1.0f, 1.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		glm::vec4 red = { 1.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 blue = { 0.0f, 0.0f, 1.0f, 1.0f };
		std::vector<glm::vec4> colors;
		colors.resize(nrParticles);

		std::for_each(std::execution::par, particles.begin(), particles.end(),
			[this, blue, &colors](uint32_t i)
		{
			colors[i] = blue;
		});

		std::vector<glm::mat4> transforms;
		transforms.resize(nrParticles);

		GLuint bufPositions/*[2]*/;
		GLuint bufColors/*[2]*/;
		GLuint bufPredictedPos;
		GLuint bufVelocities/*[2]*/;
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
		glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(glm::vec4), &Physics::Fluid::FluidSimulation::getInstace().OutPositions[0], GL_DYNAMIC_DRAW);

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

		cParticleShader.setFloat("interactionRadius", Physics::Fluid::FluidSimulation::getInstace().getInteractionRadius());
		cParticleShader.setFloat("targetDensity", Physics::Fluid::FluidSimulation::getInstace().getDensityTarget());
		cParticleShader.setFloat("pressureMultiplier", Physics::Fluid::FluidSimulation::getInstace().getPressureMultiplier());
		cParticleShader.setFloat("nearPressureMultiplier", Physics::Fluid::FluidSimulation::getInstace().getNearPressureMultiplier());
		cParticleShader.setFloat("viscosityStrength", Physics::Fluid::FluidSimulation::getInstace().getViscosityStrength());
		cParticleShader.setFloat("gravityScale", Physics::Fluid::FluidSimulation::getInstace().getGravityScale());

		cParticleShader.setVec3("boundSize", Physics::Fluid::FluidSimulation::getInstace().getBounds());
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
				Physics::PhysicsWorld::getInstace().update(deltatime);

				// MOUSE POSITION:
				//float64 x;
				//float64 y;
				//this->window->GetMousePos(x,y);
				//float xNDC = (2.0f * x) / winSize.x - 1.0f;
				//float yNDC = 1.0f - (2.0f * y) / winSize.y;

				//glm::vec4 rayClip(xNDC, yNDC, -1.0f, 1.0f);

				//glm::vec4 rayEye = Cam.GetInvProjection() * rayClip;
				//rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
				//glm::vec4 rayWorld = Cam.GetInvViewMatrix() * rayEye;
				//// --------------------------------------------------------------------------------------------
				//// Needed?
				//// --------------------------------------------------------------------------------------------
				//glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));
				//
				//glm::vec3 dir = glm::vec3(0, 0, -1);

				//glm::vec3 P = dir * 6.0f;

				//float t = glm::dot(P - Cam.Position, dir) / glm::dot(rayDirection, dir);

				//glm::vec3 hitPos = Cam.Position + t * rayDirection;
				//// --------------------------------------------------------------------------------------------
				////Update mouse world position
				//Physics::Fluid::FluidSimulation::getInstace().setMousePosition({ hitPos.x, hitPos.y });
				//
				//// Listen to Mouse press to give input strength
				//this->window->SetMousePressFunction([this](int32 button, int32 press, int32 sc)
				//{
				//	if (button == GLFW_MOUSE_BUTTON_1 && press)
				//	{
				//		printf("Mouse one pressed\n");
				//		Physics::Fluid::FluidSimulation::getInstace().setInputStrength(400);
				//	}
				//	else if (button == GLFW_MOUSE_BUTTON_2 && press)
				//	{
				//		printf("Mouse two pressed\n");
				//		Physics::Fluid::FluidSimulation::getInstace().setInputStrength(-300);
				//	}
				//	else
				//	{
				//		printf("Mouse buttons released\n");
				//		Physics::Fluid::FluidSimulation::getInstace().setInputStrength(0);
				//	}
				//});


			}
			auto simEnd = std::chrono::steady_clock::now();
			double simElapsed = std::chrono::duration<double>(simEnd - simStart).count() * 1000.0f;
			Physics::Fluid::FluidSimulation::getInstace().setSimulationTime((float)simElapsed);

			auto colorUpdateStart = std::chrono::steady_clock::now();
			// --------------------------------------------------------------------------------------------
			// Update Colors and Transition
			// --------------------------------------------------------------------------------------------
			std::for_each(std::execution::par, particles.begin(), particles.end(),
				[this, blue, red, &colors, &transforms](uint32_t i)
				{
				if (colors.size() > nrParticles || i >= nrParticles) return;
					float normalized = Physics::Fluid::FluidSimulation::getInstace().getSpeedNormalzied(i);
					colors[i] = (1.0f - normalized) * blue + normalized * red;
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

			particleShader.Enable();

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufPositions);
			glBufferData(GL_SHADER_STORAGE_BUFFER, nrParticles * sizeof(glm::vec4), &Physics::Fluid::FluidSimulation::getInstace().OutPositions[0], GL_DYNAMIC_DRAW);

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

			//for (int i = 0 ; i < nrParticles; i++)
			//{ // DRAW
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufPositions);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufColors);

			// Split drawcalls into smaller bits, since integer division on AMD cards is inaccurate
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
			//}


			particleShader.Disable();

			shader.Enable();

			shader.setMat4("view", Cam.GetViewMatrix());
			shader.setMat4("project", Cam.GetProjection());

			//BOUND
			glm::vec3 bound = Physics::Fluid::FluidSimulation::getInstace().getBounds();
			shader.setVec4("color", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
			trans = glm::translate(glm::vec3(0.0f,0.0f, 0.0f)) * glm::scale(bound);
			shader.setMat4("model", trans);
			plane3.bindVAO();
			glPolygonMode(GL_FRONT, GL_LINE);
			glPolygonMode(GL_BACK, GL_LINE);
			plane3.renderMesh(0);
			
			glPolygonMode(GL_FRONT, GL_FILL);
			glPolygonMode(GL_BACK, GL_FILL);
			plane3.unBindVAO();

			Cam.setViewProjection();
			auto renderEnd = std::chrono::steady_clock::now();
			renderingElapsed = std::chrono::duration<double>(renderEnd - renderStart).count() * 1000.0f;
			this->window->SwapBuffers();
			this->window->Update();

			auto timeEnd = std::chrono::steady_clock::now();
			deltatime = std::min(0.0333333, std::chrono::duration<double>(timeEnd - timeStart).count());
		}
		glDeleteBuffers(1, &bufPositions);
		//glDeleteBuffers(1, &bufVelocities);
		glDeleteBuffers(1, &bufColors);
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
			ImGui::Text("Particles: %i", nrParticles);
			ImGui::NewLine();
			ImGui::Text("Simulation status: %s", isRunning ? "ON" : "OFF");
			float simTime = Physics::Fluid::FluidSimulation::getInstace().getSimulationTime();
			ImGui::Text("Simulation Elapsed: %.2f ms", simTime);
			ImGui::Text("Rendering Elapsed:  %.2f ms", renderingElapsed);
			ImGui::Text("Color Elapsed:      %.2f ms", colorElapsed);
			ImGui::Text("Program Elapsed:    %.2f ms", deltatime * 1000.0f);
			ImGui::NewLine();
			int targetSphere = CurrentParticle;
			if (ImGui::InputInt("Focus Particle", &targetSphere))
			{
				if (targetSphere >= nrParticles)
				{
					targetSphere = 0;
				}
				if (targetSphere <= -1)
				{
					targetSphere = nrParticles-1;
				}
				CurrentParticle = targetSphere;
			}
			glm::vec3 pos = Physics::Fluid::FluidSimulation::getInstace().getPosition(CurrentParticle);
			glm::vec3 vel = Physics::Fluid::FluidSimulation::getInstace().getVelocity(CurrentParticle);
			ImGui::Text("  Position: (%f, %f, %f)", pos.x, pos.y, pos.z);
			ImGui::Text("  Velocity: (%f, %f, %f)", vel.x, vel.y, vel.z);
			ImGui::Text("  Density: (%f)", Physics::Fluid::FluidSimulation::getInstace().getDensity(CurrentParticle));
			ImGui::Text("  Near Density: (%f)", Physics::Fluid::FluidSimulation::getInstace().getNearDensity(CurrentParticle));

			ImGui::NewLine();
			bool gravity = Physics::Fluid::FluidSimulation::getInstace().getGravityStatus();
			if (ImGui::Checkbox("Gravity", &gravity))
			{
				Physics::Fluid::FluidSimulation::getInstace().setGravity(gravity);
			}

			ImGui::End();

			ImGui::Begin("Values");

			if (isRunning)
			{
				isRunning = !ImGui::Button("Stop", { 100,25 });
				if (ImGui::InputInt("Particles", &nrParticles, 1, 100, ImGuiInputTextFlags_ReadOnly)) {}
			}
			else
			{
				isRunning = ImGui::Button("Start", { 100,25 });
				ImGui::SameLine();
				if (ImGui::Button("Reset", { 100,25 }))
				{
					deltatime = 0.0166667f;
					Physics::Fluid::FluidSimulation::getInstace().InitializeData(nrParticles);
				}

				if (ImGui::InputInt("Particles", &nrParticles)) 
				{
					Physics::Fluid::FluidSimulation::getInstace().InitializeData(nrParticles);
				}
			}

			float interactionRadius = Physics::Fluid::FluidSimulation::getInstace().getInteractionRadius();
			if (ImGui::SliderFloat("Interaction Radius", &interactionRadius, 0.01f, 10.0f))
			{
				Physics::Fluid::FluidSimulation::getInstace().setInteractionRadius(interactionRadius);
			}

			float TargetDensity = Physics::Fluid::FluidSimulation::getInstace().getDensityTarget();
			if (ImGui::SliderFloat("Target Density", &TargetDensity, 0.0f, 100.0f))
			{
				Physics::Fluid::FluidSimulation::getInstace().setDensityTarget(TargetDensity);
			}

			float pressureMulti = Physics::Fluid::FluidSimulation::getInstace().getPressureMultiplier();
			if (ImGui::SliderFloat("Pressure Multiplier", &pressureMulti, 0.0f, 500.0f))
			{
				Physics::Fluid::FluidSimulation::getInstace().setPressureMultiplier(pressureMulti);
			}

			float nearPressureMulti = Physics::Fluid::FluidSimulation::getInstace().getNearPressureMultiplier();
			if (ImGui::SliderFloat("Pressure Near Multiplier", &nearPressureMulti, 0.0f, 100.0f))
			{
				Physics::Fluid::FluidSimulation::getInstace().setNearPressureMultiplier(nearPressureMulti);
			}

			float viscosityStrength = Physics::Fluid::FluidSimulation::getInstace().getViscosityStrength();
			if (ImGui::SliderFloat("Viscosity Strength", &viscosityStrength, 0.0f, 1.0f))
			{
				Physics::Fluid::FluidSimulation::getInstace().setViscosityStrength(viscosityStrength);
			}

			float gravityScale = Physics::Fluid::FluidSimulation::getInstace().getGravityScale();
			if (ImGui::SliderFloat("Gravity Scale", &gravityScale, 0.0f, 10.0f))
			{
				Physics::Fluid::FluidSimulation::getInstace().setGravityScale(gravityScale);
			}

			glm::vec3 bound = Physics::Fluid::FluidSimulation::getInstace().getBounds();
			int b[3] = {bound.x, bound.y, bound.z};
			if (ImGui::SliderInt3("Bounding Volume", b, 0.0f, 22.0f))
			{
				Physics::Fluid::FluidSimulation::getInstace().setBound({b[0], b[1], b[2]});
			}

			ImGui::End();
		}
	}
}