/*****************************************************************//**
 * \file   Camera.h
 * \brief  The header of a movable camera
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>		// to use glm vec
#include <vector>

class Camera
{
// Member variables:
	//glm::vec3 position{ 0.0f,0.0f,6.0f };
	glm::vec3 position{ -1, 5, 10 };
	glm::vec3 forward_direction{ 0.0f,0.0f,-1.0f };
	glm::vec3 up_direction{ 0.0f,1.0f,0.0f };

	uint32_t viewport_width = 0;
	uint32_t viewport_height = 0;

	glm::mat4 projection_matrix{ 1.0f };
	glm::mat4 inverse_projection_matrix{ 1.0f };
	
	/* Side - note: glm::mat4{1} creates a diagonal matrix with 1s on its diagonal. */

	glm::mat4 view_matrix{ 1.0f };
	glm::mat4 inverse_view_matrix{ 1.0f };

	float vertical_FOV = 45.0f;

	float near_clip_plane_distance = 0.1f;
	float far_clip_plane_distance = 100.0f;

	glm::vec2 mouse_was_at{ 0.0f,0.0f };

	std::vector<glm::vec3> ray_directions;

public:

// Constructors:
	Camera(float verticalFOV, float NearClipPlaneDistance, float FarClipPlaneDistance);

// Update Data related to the Camera:
	bool UpdateCamera(float dt);	// return whether the camera moves
	void ResizeViewport(uint32_t new_width, uint32_t new_height);

// Getters:
	float Sensitivity() const
	{
		return 0.0006f;
	}

	const glm::vec3& Position() const
	{
		return position;
	}
	const glm::vec3& ForwardDirection() const
	{
		return forward_direction;
	}

	const glm::mat4& ProjectionMatrix() const
	{
		return projection_matrix;
	}
	const glm::mat4& InverseProjectionMatrix() const
	{
		return inverse_projection_matrix;
	}

	const glm::mat4& ViewMatrix() const
	{
		return view_matrix;
	}
	const glm::mat4& InverseViewMatrix() const
	{
		return inverse_view_matrix;
	}

	const std::vector<glm::vec3>& RayDirections() const
	{
		return ray_directions;
	}

private:

	void RecomputeProjectionMatrix();
	void RecomputeViewMatrix();
	void RecomputeRayDirections();
};

#endif // !CAMERA_H