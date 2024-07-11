#pragma once

#include "key.h"

namespace InputSystem
{
	struct Keyboard
	{
		bool pressed[Key::Code::NumKeyCodes];
		bool released[Key::Code::NumKeyCodes];
		bool held[Key::Code::NumKeyCodes];
	};
}