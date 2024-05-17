#pragma once

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

#include "render/window.h"

/*
* TODO:
*	- Add functions for NanoVG
*/

namespace Game
{
	class GameApp
	{
	public:
		GameApp();
		~GameApp();

		bool Open();

		bool Run();

		bool Close();

	private:

		float deltatime = 0;
		float renderingElapsed = 0;
		float colorElapsed = 0;
		int nrParticles;
		int CurrentParticle = 0;
		bool isRunning = false;
		bool shouldReset = false;
		float64 LastX;
		float64 LastY;
		float64 MPosX;
		float64 MPosY;

		glm::vec4 Color1 = { 0.0f, 0.75f, 1.0f, 1.0f };
		glm::vec4 Color2 = { 0.0f, 1.0f, 0.0f, 1.0f };
		glm::vec4 Color3 = { 1.0f, 1.0f, 0.0f, 1.0f };
		glm::vec4 Color4 = { 1.0f, 0.0f, 0.0f, 1.0f };

		void RenderUI();

		DISPLAY::Window* window;
	};
}
