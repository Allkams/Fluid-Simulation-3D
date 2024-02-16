#pragma once

namespace Physics
{
	namespace Fluid
	{
		struct Boundary
		{
			glm::vec2 topLeft;
			glm::vec2 bottomRight;
			float width;
			float height;

			Boundary() : topLeft(1.0f), bottomRight(1.0f)
			{
				width = 2.0f;
				height = 2.0f;
			}

			Boundary(glm::vec2 inTopLeft, glm::vec2 inBottomRight) : topLeft(inTopLeft), bottomRight(inBottomRight) 
			{
				width = abs(inTopLeft.x) + abs(inBottomRight.x);
				height = abs(inTopLeft.y) + abs(inBottomRight.y);
			}
		};
	}
}