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

#include "window.h"

namespace DISPLAY
{
	Window::Window() : window(nullptr), width(800), height(600)
	{
		glfwInit();
	}

	Window::~Window()
	{
		//Empty
	}

	bool Window::Open()
	{
		this->window = glfwCreateWindow(this->width, this->height, "Spectrawise-Engine", NULL, NULL);

		if (this->window == nullptr)
		{
			printf("[ WINDOW ] : Failed to create GLFW Window! \n");
			glfwTerminate();
			return false;
		}

		glfwMakeContextCurrent(this->window);

		glViewport(0, 0, this->width, this->height);

		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		}
		fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
		fprintf(stdout, "Status: Using OpenGL %s\n", glGetString(GL_VERSION));

		return true;
	}

	bool Window::Close()
	{
		glfwSetWindowShouldClose(this->window, true);
		glfwTerminate();
		return 0;
	}

	const bool Window::IsOpen()
	{
		return !glfwWindowShouldClose(this->window);
	}

	bool Window::ProcessInput(int32 key)
	{
		return (glfwGetKey(this->window, key) == GLFW_PRESS);
	}

	void Window::Update()
	{
		glfwSwapBuffers(this->window);
		glfwPollEvents();
	}
}