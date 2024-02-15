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
#include <imgui.h>
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

namespace DISPLAY
{

#ifdef __WIN32__
#define APICALLTYPE __stdcall
#else
#define APICALLTYPE
#endif

	static void GLAPIENTRY
		GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		std::string msg("[OPENGL DEBUG MESSAGE] ");

		// print error severity
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_LOW:
			msg.append("<Low severity> ");
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			msg.append("<Medium severity> ");
			break;
		case GL_DEBUG_SEVERITY_HIGH:
			msg.append("<High severity> ");
			break;
		}

		// append message to output
		msg.append(message);

		// print message
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			printf("Error: %s\n", msg.c_str());
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			printf("Performance issue: %s\n", msg.c_str());
			break;
		default:		// Portability, Deprecated, Other
			break;
		}
	}
	
	int32 Window::WindowCount = 0;

	Window::Window() : window(nullptr), width(1280), height(720)
	{
		glfwInit();
	}

	Window::~Window()
	{
		//Empty
	}

	bool Window::Open()
	{
		if (Window::WindowCount == 0)
		{
			if (!glfwInit()) return false;
		}

		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glfwWindowHint(GLFW_RED_BITS, 8);
		glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS, 8);
		glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

		this->window = glfwCreateWindow(this->width, this->height, "Spectrawise-Engine", NULL, NULL);

		if (this->window == nullptr)
		{
			printf("[ WINDOW ] : Failed to create GLFW Window! \n");
			glfwTerminate();
			return false;
		}

		glfwMakeContextCurrent(this->window);

		if (nullptr != this->window && WindowCount == 0)
		{
			GLenum res = glewInit();
			assert(res == GLEW_OK);

			const GLubyte* vendor = glGetString(GL_VENDOR);
			const GLubyte* renderer = glGetString(GL_RENDERER);
			printf("GPU Vendor: %s\n", vendor);
			printf("GPU Render Device: %s\n", renderer);

			if (!(GLEW_VERSION_4_0))
			{
				printf("[WARNING]: OpenGL 4.0+ is not supported on this hardware!\n");
				glfwDestroyWindow(this->window);
				this->window = nullptr;
				return false;
			}

			// setup debug callback
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(GLDebugCallback, NULL);
			GLuint unusedIds;
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);

			// setup stuff
			//glEnable(GL_FRAMEBUFFER_SRGB);
			glEnable(GL_MULTISAMPLE);

			// disable vsync
			glfwSwapInterval(0);

			// setup viewport
			glViewport(0, 0, this->width, this->height);
		}

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = { (float)width, (float)height };
		io.DeltaTime = 1 / 60.0f;
		//io.PixelCenterOffset = 0.0f;
		//io.FontTexUvForWhite = ImVec2(1, 1);
		//io.RenderDrawListsFn = ImguiDrawFunction;

		ImGui_ImplGlfw_InitForOpenGL(this->window, false);
		ImGui_ImplOpenGL3_Init();

		// load default font
		ImFontConfig config;
		config.OversampleH = 3;
		config.OversampleV = 1;
#if _WIN32
		ImFont* font = io.Fonts->AddFontFromFileTTF("c:/windows/fonts/tahoma.ttf", 14, &config);
#else
		ImFont* font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 18, &config);
#endif

		unsigned char* buffer;
		int width, height, channels;
		io.Fonts->GetTexDataAsRGBA32(&buffer, &width, &height, &channels);

		glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);

		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		//Input::InputHandler::Create();

		// increase window count and return result
		Window::WindowCount++;

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
		if (nullptr != this->window) glfwDestroyWindow(this->window);
		this->window = nullptr;
		Window::WindowCount--;
		if (Window::WindowCount == 0)
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			glfwTerminate();
		}
		return true;
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
		//glfwSwapBuffers(this->window);
		glfwPollEvents();
	}

	void Window::SwapBuffers()
	{
		if (this->window)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();

			ImGui::NewFrame();
			if (nullptr != this->uiFunc)
				this->uiFunc();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(this->window);
		}
	}

	void Window::setSize(int32 width, int32 height)
	{
		reSize(width, height);
		this->width = width;
		this->height = height;
	}
	void Window::setSize(glm::vec2 size)
	{
		reSize(size.x, size.y);
		this->width = size.x;
		this->height = size.y;
	}
	glm::vec2 Window::getSize() { return glm::vec2(this->width, this->height); }

	void Window::setTitle(std::string title)
	{
		this->title = title;
		if (nullptr != this->window) glfwSetWindowTitle(this->window, this->title.c_str());
	}

	std::string Window::getTitle()
	{
		return this->title;
	}

	void Window::reSize(int32 width, int32 height)
	{
		if (nullptr != this->window)
		{
			glfwSetWindowSize(this->window, width, height);
			glViewport(0, 0, this->width, this->height);
		}
	}
}