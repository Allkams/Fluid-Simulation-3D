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

#include <iostream>
#include <GL/glew.h>
#include <glm.hpp>

namespace RenderUtils
{
	enum CameraMovement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		DOWN,
		UP
	};

	class Camera
	{
	public:
		glm::vec3 Position;
		glm::vec3 ForwardVector;
		glm::vec3 UpVector;
		glm::vec3 RightVector;
		glm::vec3 WorldUpVector;
		glm::vec3 Target;
		bool shouldTarget = false;


		float Yaw;
		float Pitch;

		float MovementSpeed = 5.0f;
		float MouseSensitivity;
		float FOV = 45.0f;

		Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);
		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

		void setViewMatrix(bool FocusTarget = false);
		glm::mat4 GetViewMatrix();
		glm::mat4 GetInvViewMatrix();
		void setProjection(float width, float height, float near, float far);
		glm::mat4 GetProjection();
		glm::mat4 GetInvProjection();
		glm::mat4 GetViewProjection();
		glm::mat4 GetInvViewProjection();
		void setViewProjection();

		void Move(CameraMovement direction, float deltaTime);
		void Look(float xOffset, float yOffset, GLboolean constrainPitch = true);
		void Zoom(float yOffset);
	private:
		glm::mat4 View;
		glm::mat4 InvView;
		glm::mat4 Projection;
		glm::mat4 InvProjection;
		glm::mat4 ViewProjection;
		glm::mat4 InvViewProjection;
		void updateCamera();
	};
}