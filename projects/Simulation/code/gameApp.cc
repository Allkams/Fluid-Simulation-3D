// 
// Copyright 2023 Alexander Marklund (Allkams02@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand associated
// documentation files(the �Software�), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and /or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
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

		glm::vec2 winSize = window->getSize();
		//PreWork
		RenderUtils::Camera Cam(glm::vec3(0));
		nrParticles = 64*64;
		Physics::Fluid::FluidSimulation::getInstace().InitializeData(nrParticles);
		std::vector<uint32_t> particles;
		for (int i = 0; i < nrParticles; i++)
		{
			particles.push_back(i);
		}

		Shader shader = Shader("./shaders/VertexShader.vs", "./shaders/FragementShader.fs");
		shader.Enable();

		glm::mat4 perspect = Cam.GetPerspective(winSize.x, winSize.y, 0.1f, 1000.0f);

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::translate(glm::vec3(0.0f, 0.0f, -3.0f));

		glm::mat4 view = Cam.GetViewMatrix();

		shader.setMat4("model", trans);
		shader.setMat4("view", view);
		shader.setMat4("project", perspect);
		Render::simpleMesh circle = Render::CreateSimpleCircle(0.05f, 12);
		Render::Mesh plane2 = Render::CreateCircle(0.35f, 12);
		//Solve this problem...
		Render::Mesh plane3 = Render::CreatePlane(1.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		glm::vec4 red = { 1.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 blue = { 0.0f, 0.0f, 1.0f, 1.0f };
		std::vector<glm::vec4> colors;
		colors.resize(nrParticles);

		std::vector<glm::mat4> transforms;
		transforms.resize(nrParticles);

		deltatime = 0.016667f;
		while (this->window->IsOpen())
		{
			auto timeStart = std::chrono::steady_clock::now();
			glClearColor(0.4f, 0.0f, 0.8f, 1.0f);

			colors.resize(nrParticles);
			transforms.resize(nrParticles);

			// Accept fragment if it closer to the camera than the former one
			glDepthFunc(GL_LESS);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (this->window->ProcessInput(GLFW_KEY_F1))
			{
				shader.ReloadShader();
				shader.Enable();
				shader.setMat4("transform", view);
			}

			if (this->window->ProcessInput(GLFW_KEY_ESCAPE))
			{
				this->window->Close();
				break;
			}

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
				Physics::PhysicsWorld::getInstace().update(deltatime);
			}
			auto simEnd = std::chrono::steady_clock::now();
			double simElapsed = std::chrono::duration<double>(simEnd - simStart).count() * 1000.0f;
			Physics::Fluid::FluidSimulation::getInstace().setSimulationTime((float)simElapsed);

			auto colorUpdateStart = std::chrono::steady_clock::now();
			std::for_each(std::execution::par, particles.begin(), particles.end(),
				[this, blue, red, &colors, &transforms](uint32_t i)
				{
				if (colors.size() > nrParticles || i >= nrParticles) return;
					float normalized = Physics::Fluid::FluidSimulation::getInstace().getSpeedNormalzied(i);
					glm::vec4 color = (1.0f - normalized) * blue + normalized * red;
					colors[i] = color;

					transforms[i] = glm::translate(glm::vec3(Physics::Fluid::FluidSimulation::getInstace().getPosition(i), -3.0f));
				});
			auto colorUpdateEnd = std::chrono::steady_clock::now();
			colorElapsed = std::chrono::duration<double>(colorUpdateEnd - colorUpdateStart).count() * 1000.0f;

			auto renderStart = std::chrono::steady_clock::now();
			circle.bindVAO();
			for (int i = 0; i < nrParticles; i++)
			{
				shader.setVec4("color", colors[i]);
				shader.setMat4("model", transforms[i]);
				circle.renderMesh();

			}
			circle.unBindVAO();

			shader.setVec4("color", glm::vec4(0.3f, 0.0f, 0.0f, 0.2f));
			glm::mat4 trans = glm::translate(glm::vec3(Physics::Fluid::FluidSimulation::getInstace().getPosition(CurrentParticle), -3.0f));
			shader.setMat4("model", trans);
			plane2.bindVAO();
			plane2.renderMesh(0);
			plane2.unBindVAO();

			//BOUND
			glm::vec2 bound = Physics::Fluid::FluidSimulation::getInstace().getBounds();
			shader.setVec4("color", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
			trans = glm::translate(glm::vec3(0.0f,0.0f, -3.0f)) * glm::scale(glm::vec3(bound, 0.0f));
			shader.setMat4("model", trans);

			plane3.bindVAO();
			plane3.renderMesh(0);
			plane3.unBindVAO();
			auto renderEnd = std::chrono::steady_clock::now();
			renderingElapsed = std::chrono::duration<double>(renderEnd - renderStart).count() * 1000.0f;
			this->window->SwapBuffers();
			this->window->Update();

			auto timeEnd = std::chrono::steady_clock::now();
			deltatime = std::min(0.0333333, std::chrono::duration<double>(timeEnd - timeStart).count());
		}

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
			ImGui::Text("  Position: (%f, %f)", Physics::Fluid::FluidSimulation::getInstace().getPosition(CurrentParticle).x, Physics::Fluid::FluidSimulation::getInstace().getPosition(CurrentParticle).y);
			ImGui::Text("  Velocity: (%f, %f)", Physics::Fluid::FluidSimulation::getInstace().getVelocity(CurrentParticle).x, Physics::Fluid::FluidSimulation::getInstace().getVelocity(CurrentParticle).y);
			ImGui::Text("  Density: (%f)", Physics::Fluid::FluidSimulation::getInstace().getDensity(CurrentParticle));

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
			if (ImGui::SliderFloat("Pressure Multiplier", &pressureMulti, 0.0f, 10.0f))
			{
				Physics::Fluid::FluidSimulation::getInstace().setPressureMultiplier(pressureMulti);
			}

			float nearPressureMulti = Physics::Fluid::FluidSimulation::getInstace().getNearPressureMultiplier();
			if (ImGui::SliderFloat("Pressure Near Multiplier", &nearPressureMulti, 0.0f, 10.0f))
			{
				Physics::Fluid::FluidSimulation::getInstace().setNearPressureMultiplier(nearPressureMulti);
			}

			float viscosityStrength = Physics::Fluid::FluidSimulation::getInstace().getViscosityStrength();
			if (ImGui::SliderFloat("Viscosity Strength", &viscosityStrength, 0.0f, 0.1f))
			{
				Physics::Fluid::FluidSimulation::getInstace().setViscosityStrength(viscosityStrength);
			}

			float gravityScale = Physics::Fluid::FluidSimulation::getInstace().getGravityScale();
			if (ImGui::SliderFloat("Gravity Scale", &gravityScale, 0.0f, 10.0f))
			{
				Physics::Fluid::FluidSimulation::getInstace().setGravityScale(gravityScale);
			}

			glm::vec2 bound = Physics::Fluid::FluidSimulation::getInstace().getBounds();
			int b[2] = {bound.x, bound.y};
			if (ImGui::SliderInt2("Bounding Volume", b, 0.0f, 20.0f))
			{
				Physics::Fluid::FluidSimulation::getInstace().setBound({b[0], b[1] });
			}

			ImGui::End();
		}
	}
}