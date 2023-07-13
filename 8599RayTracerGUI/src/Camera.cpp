/*****************************************************************//**
 * \file   Camera.cpp
 * \brief  The definitions of a movable camera
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>	// to use glm::quat
#include <glm/gtx/quaternion.hpp>	// to use glm::rotate

#include "Walnut/Input/Input.h"
#include "Walnut/Random.h"

Camera::Camera(float verticalFOV, float NearClipPlaneDistance, float FarClipPlaneDistance)
	: vertical_FOV{ verticalFOV }, near_clip_plane_distance{ NearClipPlaneDistance }, far_clip_plane_distance{ FarClipPlaneDistance }
{

}

bool Camera::UpdateCamera(float dt)
{
	// now we want to recompute ray directions when we are accumulating the temporal data, since for every frame we are doing random sampling inside each pixel.
	RecomputeViewMatrix();		// Note: order matters!
	RecomputeRayDirections();

	glm::vec2 mouse_currently_at = Walnut::Input::GetMousePosition();
	glm::vec2 mouse_displacement = mouse_currently_at - mouse_was_at;
	mouse_was_at = mouse_currently_at;

	if (!Walnut::Input::IsMouseButtonDown(Walnut::MouseButton::Right))
	{
		Walnut::Input::SetCursorMode(Walnut::CursorMode::Normal);
		return false;
	}
	Walnut::Input::SetCursorMode(Walnut::CursorMode::Locked);

	bool is_moved{ false };
	float moving_speed{ 5.0f };
	glm::vec3 right_direction{ glm::cross(forward_direction, up_direction) };

	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::W))
	{
		position += moving_speed * dt * forward_direction;
		is_moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::S))
	{
		position -= moving_speed * dt * forward_direction;
		is_moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::D))
	{
		position += moving_speed * dt * right_direction;
		is_moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::A))
	{
		position -= moving_speed * dt * right_direction;
		is_moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::Space))
	{
		position += moving_speed * dt * up_direction;
		is_moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::LeftShift))
	{
		position -= moving_speed * dt * up_direction;
		is_moved = true;
	}
	if ((mouse_displacement.x != 0.0f) || (mouse_displacement.y != 0.0f))
	{
		float change_in_pitch{ mouse_displacement.y * Sensitivity() };
		float change_in_yaw{ mouse_displacement.x * Sensitivity() };
		glm::quat quaternion{ glm::normalize(glm::cross(glm::angleAxis(-change_in_pitch, right_direction), glm::angleAxis(-change_in_yaw, up_direction))) };
		forward_direction = glm::rotate(quaternion, forward_direction);
		is_moved = true;
	}

	return is_moved;
}

void Camera::ResizeViewport(uint32_t new_width, uint32_t new_height)
{
	if ((viewport_width == new_width) && (viewport_height == new_height))
	{
		return;
	}
	viewport_width = new_width;
	viewport_height = new_height;

	RecomputeProjectionMatrix();		// Note: order matters!
	RecomputeRayDirections();
}

void Camera::RecomputeProjectionMatrix()
{
	projection_matrix = glm::perspectiveFov(glm::radians(vertical_FOV), (float)viewport_width, (float)viewport_height, near_clip_plane_distance, far_clip_plane_distance);
	/* passing in viewport_width and viewport_height is to calculate the aspect ratio */
	inverse_projection_matrix = glm::inverse(projection_matrix);
}

void Camera::RecomputeViewMatrix()
{
	view_matrix = glm::lookAt(position, position + forward_direction, up_direction);
	inverse_view_matrix = glm::inverse(view_matrix);
}

void Camera::RecomputeRayDirections()
{
	ray_directions.resize(viewport_width * viewport_height);
	for (uint32_t y = 0; y < viewport_height; y++)
	{
		for (uint32_t x = 0; x < viewport_width; x++)
		{
			glm::vec2 coordinate{ ((float)x + Walnut::Random::Float()) / viewport_width, ((float)y + Walnut::Random::Float()) / viewport_height };		// used to be bottom-left corner of the pixel, now we want one random sampling inside each pixel for every frame.
			//glm::vec2 coordinate{ (float)x / viewport_width, (float)y / viewport_height };		// bottom-left corner of the pixel
			coordinate = coordinate * 2.0f - 1.0f;		// normalize to [-1,1)^2

			// Get the ray direction in world space (from the camera to the pixel on the near clip plane of the perspective projection):
			glm::vec4 target{ inverse_projection_matrix * glm::vec4{coordinate.x, coordinate.y, 1, 1} };
			glm::vec3 ray_direction{ glm::vec3{inverse_view_matrix * glm::vec4{glm::normalize(glm::vec3{target} / target.w),0}} };

			ray_directions[y * viewport_width + x] = ray_direction;
		}
	}
}