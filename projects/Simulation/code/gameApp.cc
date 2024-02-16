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
			this->window->setSize(1920, 1080);
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
		nrParticles = 1024;
		Physics::Fluid::FluidSimulation::getInstace().InitializeData(nrParticles);

		Shader shader = Shader("./shaders/VertexShader.vs", "./shaders/FragementShader.fs");
		shader.Enable();

		glm::mat4 perspect = Cam.GetPerspective(winSize.x, winSize.y, 0.1f, 1000.0f);

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::translate(glm::vec3(0.0f, 0.0f, -3.0f));

		glm::mat4 view = Cam.GetViewMatrix();

		shader.setMat4("model", trans);
		shader.setMat4("view", view);
		shader.setMat4("project", perspect);
		Render::Mesh plane = Render::CreateCircle(0.05f, 12);
		Render::Mesh plane2 = Render::CreateCircle(0.35f, 12);
		Render::Mesh plane3 = Render::CreatePlane(12, 8);

		bool running = false;

		glEnable(GL_DEPTH_TEST);

		glm::vec4 red = { 1.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 blue = { 0.0f, 0.0f, 1.0f, 1.0f };
		std::vector<glm::vec4> colors;
		colors.resize(nrParticles);

		deltatime = 0.016667f;
		while (this->window->IsOpen())
		{
			auto timeStart = std::chrono::steady_clock::now();
			glClearColor(0.4f, 0.0f, 0.8f, 1.0f);

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

			if (this->window->ProcessInput(GLFW_KEY_SPACE))
			{
				running = !running;
			}

			if (running)
			{
				Physics::PhysicsWorld::getInstace().update(deltatime);
			}

			for (int i = 0; i < nrParticles; i++)
			{
				float normalized = Physics::Fluid::FluidSimulation::getInstace().getSpeedNormalzied(i);
				glm::vec4 color = (1.0f - normalized) * blue + normalized * red;
				colors[i] = color;
			}

			for (int i = 0; i < nrParticles; i++)
			{
				shader.setVec4("color", colors[i]);

				glm::mat4 trans = glm::translate(glm::vec3(Physics::Fluid::FluidSimulation::getInstace().getPosition(i), -3.0f));

				shader.setMat4("model", trans);
				plane.bindVAO();
				plane.renderMesh(0);
				plane.unBindVAO();
			}

			shader.setVec4("color", glm::vec4(0.3f, 0.0f, 0.0f, 0.2f));
			glm::mat4 trans = glm::translate(glm::vec3(Physics::Fluid::FluidSimulation::getInstace().getPosition(CurrentParticle), -3.0f));
			shader.setMat4("model", trans);
			plane2.bindVAO();
			plane2.renderMesh(0);
			plane2.unBindVAO();

			//BOUND
			shader.setVec4("color", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
			trans = glm::translate(glm::vec3(0.0f,0.0f, -3.0f));
			shader.setMat4("model", trans);

			plane3.bindVAO();
			plane3.renderMesh(0);
			plane3.unBindVAO();

			this->window->SwapBuffers();
			this->window->Update();

			auto timeEnd = std::chrono::steady_clock::now();
			deltatime = std::chrono::duration<double>(timeEnd - timeStart).count();
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
			ImGui::Begin("Debug");

			int fps = 1.0f/ deltatime;
			ImGui::Text("FPS: %i", fps);
			ImGui::Text("Particles: %i", nrParticles);
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


			float ForceMulti = 0.0;
			if (ImGui::SliderFloat("DEMO", &ForceMulti, 0.0f, 100.0f))
			{
				
			}

			ImGui::End();
		}
	}
}