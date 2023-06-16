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
#include "BoundingVolume.h"
#include "IntersectionRecord.h"

namespace Whitted
{
	class Entity
	{
	public:
		Entity()
		{

		}

		virtual ~Entity()
		{

		}

		virtual AccelerationStructure::AABB_3D Get3DAABB() = 0;

		virtual glm::vec3 GetDiffuseColor(const glm::vec2& texture_coordinates = glm::vec2{ 0.0f,0.0f }) const = 0;

		virtual bool Intersect(const AccelerationStructure::Ray& ray)
		{
			return true;
		}

		virtual bool Intersect(const AccelerationStructure::Ray& ray, float& t_intersection, uint32_t& triangle_index) const
		{
			return false;
		}

		virtual IntersectionRecord GetIntersectionRecord(AccelerationStructure::Ray ray) = 0;

		virtual void GetHitInfo(
			const glm::vec3& intersection, 
			const glm::vec3& light_direction, 
			const uint32_t& triangle_index, 
			const glm::vec2& barycentric_coordinates, 
			glm::vec3& surface_normal, 
			glm::vec2& texture_coordinates
		) const = 0;

	public:
		// Currently NO data member for this class
	};
}

#endif // !ENTITY_H