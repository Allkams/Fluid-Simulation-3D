#pragma once

#include "mouse.h"
#include "keyboard.h"
//#include "gamepad.h" //Not implemented yet..
#include <vector>

namespace InputSystem
{
	class InputHandler
	{
		InputHandler() = delete;
		~InputHandler() = delete;

	public:
		static void CreateHandler();
		static void BeginFrame();

		static void HandleKeyEvent(int32 key, int32 scancode, int32 action, int32 mods);
		static void HandleMousePressEvent(int32 button, int32 pressed, int32 mods);
		static void HandleMouseMoveEvent(double x, double y);
	};

	Keyboard* GetDefaultKeyboard();
	Mouse* GetDefaultMouse();
}