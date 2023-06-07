/*****************************************************************//**
 * \file   Entity.h
 * \brief  The base class for any entity in the scene to be ray-traced in Whitted-Style
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef ENTITY_H
#define ENTITY_H

#include "WhittedUtilities.h"
#include "VectorFloat.h"

namespace Whitted
{
	class Entity
	{
	public:
		Entity()
		{

		}

		virtual ~Entity() = default;

		virtual glm::vec3 GetDiffuseColor(const glm::vec2& texture_coordinates) const
		{
			return diffuse_color;
		}

		virtual bool Intersect(
			const glm::vec3& light_origin, 
			const glm::vec3& light_direction, 
			float& closerT,
			uint32_t& triangle_index, 
			glm::vec2& barycentric_coordinates
		) const = 0;

		virtual void GetHitInfo(
			const glm::vec3& intersection, 
			const glm::vec3& light_direction, 
			const uint32_t& triangle_index, 
			const glm::vec2& barycentric_coordinates, 
			glm::vec3& surface_normal, 
			glm::vec2& texture_coordinates
		) const = 0;

	public:
		MaterialNature material_nature{ Diffuse_Glossy };
		float refractive_index{ 1.3f };
		float phong_diffuse{ 0.8f };
		float phong_specular{ 0.2f };
		glm::vec3 diffuse_color{0.2f, 0.2f, 0.2f};
		float specular_size_factor{ 25.0f };	// The larger this factor is, the SMALLER the specular size will be.
												// Because this will be used as an exponent.
	};
}

#endif // !ENTITY_H