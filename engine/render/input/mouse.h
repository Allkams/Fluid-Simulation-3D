#pragma once
#include <config.cc>

namespace InputSystem
{
	struct Mouse
	{
		enum ButtonKeys
		{
			LeftButtonKey = 0,
			RightButtonKey = 1,
			MiddleButtonKey = 2,
			NumButtonKeys,
		};

		bool pressed[ButtonKeys::NumButtonKeys];
		bool released[ButtonKeys::NumButtonKeys];
		bool held[ButtonKeys::NumButtonKeys];

		glm::vec2 position;

		glm::vec2 delta;

		glm::vec2 previousPosition;
	};
}