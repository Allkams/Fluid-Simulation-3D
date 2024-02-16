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

		void SetKeyPressFunction(const std::function<void(int32, int32, int32, int32)>& func);
		/// set mouse press function callback
		void SetMousePressFunction(const std::function<void(int32, int32, int32)>& func);
		/// set mouse move function callback
		void SetMouseMoveFunction(const std::function<void(double, double)>& func);
		/// set mouse enter leave function callback
		void SetMouseEnterLeaveFunction(const std::function<void(bool)>& func);
		/// set mouse scroll function callback
		void SetMouseScrollFunction(const std::function<void(double, double)>& func);
		/// set window resize function callback
		void SetWindowResizeFunction(const std::function<void(int32, int32)>& func);

		void SetUiRender(const std::function<void()>& func);

		void GetMousePos(float64& x, float64& y);

	private:

		/// static key press callback
		static void StaticKeyPressCallback(GLFWwindow* win, int32 key, int32 scancode, int32 action, int32 mods);
		/// static mouse press callback
		static void StaticMousePressCallback(GLFWwindow* win, int32 button, int32 action, int32 mods);
		/// static mouse move callback
		static void StaticMouseMoveCallback(GLFWwindow* win, float64 x, float64 y);
		/// static mouse enter/leave callback
		static void StaticMouseEnterLeaveCallback(GLFWwindow* win, int32 mode);
		/// static mouse scroll callback
		static void StaticMouseScrollCallback(GLFWwindow* win, float64 x, float64 y);
		/// static resize window callback
		static void StaticWindowResizeCallback(GLFWwindow* win, int32 x, int32 y);

		static void StaticCloseCallback(GLFWwindow* window);
		static void StaticFocusCallback(GLFWwindow* window, int focus);
		static void StaticCharCallback(GLFWwindow* window, unsigned int key);
		static void StaticDropCallback(GLFWwindow* window, int files, const char** args);

		void reSize(int32 width, int32 height);

		static int32 WindowCount;

		std::function<void(int32, int32, int32, int32)> keyPressCallback;
		/// function for mouse press callbacks
		std::function<void(int32, int32, int32)> mousePressCallback;
		/// function for mouse move callbacks
		std::function<void(double, double)> mouseMoveCallback;
		/// function for mouse enter/leave callbacks
		std::function<void(bool)> mouseLeaveEnterCallback;
		/// function for mouse scroll callbacks
		std::function<void(double, double)> mouseScrollCallback;
		/// function for window resize callbacks
		std::function<void(int32, int32)> windowResizeCallback;

		std::function<void()> uiFunc;

		int32 width;
		int32 height;
		std::string title;
		GLFWwindow* window;
	};

	inline void
		Window::SetKeyPressFunction(const std::function<void(int32, int32, int32, int32)>& func)
	{
		this->keyPressCallback = func;
	}

	//------------------------------------------------------------------------------
	/**
		parameters:
			button code
			pressed (bool)
			scancode?
	*/
	inline void
		Window::SetMousePressFunction(const std::function<void(int32, int32, int32)>& func)
	{
		this->mousePressCallback = func;
	}

	//------------------------------------------------------------------------------
	/**
		parameters: x, y position
	*/
	inline void
		Window::SetMouseMoveFunction(const std::function<void(double, double)>& func)
	{
		this->mouseMoveCallback = func;
	}

	//------------------------------------------------------------------------------
	/**
	*/
	inline void
		Window::SetMouseEnterLeaveFunction(const std::function<void(bool)>& func)
	{
		this->mouseLeaveEnterCallback = func;
	}

	//------------------------------------------------------------------------------
	/**
	*/
	inline void
		Window::SetMouseScrollFunction(const std::function<void(double, double)>& func)
	{
		this->mouseScrollCallback = func;
	}

	//------------------------------------------------------------------------------
	/**
	*/
	inline void
		Window::SetWindowResizeFunction(const std::function<void(int32, int32)>& func)
	{
		this->windowResizeCallback = func;
	}

	inline void Window::SetUiRender(const std::function<void()>& func)
	{
		this->uiFunc = func;
	}
}