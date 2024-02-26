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

#include "config.h"
#include "camera.h"
#include "../glm/gtc/matrix_transform.hpp"

namespace RenderUtils
{
	Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : ForwardVector(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f), FOV(45.0f)
	{
		Position = position;
		WorldUpVector = up;
		Yaw = yaw;
		Pitch = pitch;
		Target = ForwardVector * 2.0f;
		updateCamera();
	}

	Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : ForwardVector(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f), FOV(45.0f)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUpVector = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		Target = ForwardVector * 2.0f;
		updateCamera();
	}

	void Camera::setViewMatrix()
	{
		View = glm::lookAt(Position, Position + ForwardVector, glm::vec3(0, 1.0f, 0));
		InvView = glm::inverse(View);
		setViewProjection();
	}

	glm::mat4 Camera::GetViewMatrix()
	{
		return View;
	}

	glm::mat4 Camera::GetInvViewMatrix()
	{
		return InvView;
	}

	void Camera::setProjection(float width, float height, float near, float far)
	{
		Projection = glm::perspective(glm::radians(90.0f), width / height, near, far);
		InvProjection = glm::inverse(Projection);
		setViewProjection();
	}

	glm::mat4 Camera::GetProjection()
	{
		return Projection;
	}

	glm::mat4 Camera::GetInvProjection()
	{
		return InvProjection;
	}

	glm::mat4 Camera::GetViewProjection()
	{
		return ViewProjection;
	}

	glm::mat4 Camera::GetInvViewProjection()
	{
		return InvViewProjection;
	}

	void Camera::setViewProjection()
	{
		this->ViewProjection = this->Projection * this->View;
		this->InvViewProjection = glm::inverse(ViewProjection);
	}

	void Camera::Move(CameraMovement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += ForwardVector * velocity;
		if (direction == BACKWARD)
			Position -= ForwardVector * velocity;
		if (direction == LEFT)
			Position -= RightVector * velocity;
		if (direction == RIGHT)
			Position += RightVector * velocity;

		updateCamera();
	}

	void Camera::Look(float xoffset, float yoffset, GLboolean constrainPitch)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		updateCamera();
	}

	void Camera::Zoom(float yoffset)
	{
		FOV -= (float)yoffset;
		if (FOV < 1.0f)
			FOV = 1.0f;
		if (FOV > 45.0f)
			FOV = 45.0f;
	}

	void Camera::updateCamera()
	{
		// calculate the new forward vector
		glm::vec3 forwardVec = glm::vec3(0);
		forwardVec.x = cos(glm::radians(Yaw)) * sin(glm::radians(Pitch));
		forwardVec.y = sin(glm::radians(Pitch));
		forwardVec.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		ForwardVector = glm::normalize(forwardVec);
		// also re-calculate the Right and Up vector
		RightVector = glm::normalize(glm::cross(ForwardVector, WorldUpVector));
		UpVector = glm::normalize(glm::cross(RightVector, ForwardVector));
	}
}