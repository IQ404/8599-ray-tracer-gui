/*****************************************************************//**
 * \file   Sphere.h
 * \brief  The representation of spheres to be Whitted-style ray-traced 
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef SPHERE_H
#define SPHERE_H

#include "VectorFloat.h"
#include "Entity.h"

namespace Whitted
{
	class Sphere : public Entity
	{
	public:
		Sphere(const glm::vec3& center, const float& radius)
			: m_center{ center }, m_radius{ radius }, radius_squared{ radius * radius }
		{

		}

		virtual bool Intersect(
			const glm::vec3& light_origin,
			const glm::vec3& light_direction,
			float& closerT,
			uint32_t&,
			glm::vec2&
		) const override
		{
			float t_small;
			float t_large;
			glm::vec3 center_to_light_origin = light_origin - m_center;
			
			if (!QuadraticFormula(
				glm::dot(light_direction, light_direction),
				2 * glm::dot(light_direction, center_to_light_origin),
				glm::dot(center_to_light_origin, center_to_light_origin) - radius_squared,
				t_small,
				t_large)
				)
			{
				return false;
			}

			if (t_small < 0.0f)
			{
				t_small = t_large;
			}
			if (t_small < 0.0f)
			{
				return false;
			}
			closerT = t_small;
			return true;
		}

		virtual void GetHitInfo(
			const glm::vec3& intersection,
			const glm::vec3&,
			const uint32_t&,
			const glm::vec2&,
			glm::vec3& surface_normal,
			glm::vec2&
		) const override
		{
			surface_normal = Whitted::normalize(intersection - m_center);
		}


	private:
		glm::vec3 m_center;
		float m_radius;
		float radius_squared;	// to reduce computation when calculating intersections
	};
}

#endif // !SPHERE_H
