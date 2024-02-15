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

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include <functional>

/*
* TODOS:
*	- Add SetSize
*	- Add GetSize
*	- Add a Title variable
*	- Add Setter and Getter for title.
*	- More inputs.
*/

namespace DISPLAY
{
	class Window
	{
	public:
		Window();
		~Window();

		bool Open();
		bool Close();
		const bool IsOpen();

		void Update();
		void SwapBuffers();

		bool ProcessInput(int32 key);

		void setSize(int32 width, int32 height);
		void setSize(glm::vec2 size);
		glm::vec2 getSize();

		void setTitle(std::string title);
		std::string getTitle();

		void SetUiRender(const std::function<void()>& func);

	private:

		void reSize(int32 width, int32 height);

		static int32 WindowCount;

		std::function<void()> uiFunc;

		int32 width;
		int32 height;
		std::string title;
		GLFWwindow* window;
	};

	inline void Window::SetUiRender(const std::function<void()>& func)
	{
		this->uiFunc = func;
	}
}