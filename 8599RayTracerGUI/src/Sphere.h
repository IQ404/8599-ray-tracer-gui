/*****************************************************************//**
 * \file   Sphere.h
 * \brief  The representation of spheres to be Whitted-style ray-traced 
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef SPHERE_H
#define SPHERE_H

#include "Entity.h"

namespace Whitted
{
	class Sphere : public Entity
	{
	public:
		Sphere(const glm::vec3& center, const float& radius)
			: m_center{ center }, m_radius{ radius }, radius_squared{ radius * radius }, material{ new WhittedMaterial() }
		{

		}

		virtual glm::vec3 GetDiffuseColor(const glm::vec2&) const override
		{
			return material->GetDiffuseColor();
		}

		virtual AccelerationStructure::AABB_3D Get3DAABB() override
		{
			return AccelerationStructure::AABB_3D{m_center + m_radius, m_center - m_radius };
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

		virtual IntersectionRecord GetIntersectionRecord(AccelerationStructure::Ray ray) override
		{
			IntersectionRecord record;		// default-initialized to "has_intersection = false"
			float t_small;
			float t_large;
			glm::vec3 center_to_light_origin = ray.m_origin - m_center;

			if (!QuadraticFormula(
				glm::dot(ray.m_direction, ray.m_direction),
				2 * glm::dot(ray.m_direction, center_to_light_origin),
				glm::dot(center_to_light_origin, center_to_light_origin) - radius_squared,
				t_small,
				t_large)
				)
			{
				return record;
			}

			if (t_small < 0.0f)
			{
				t_small = t_large;
			}
			if (t_small < 0.0f)
			{
				return record;
			}

			record.has_intersection = true;
			record.t = t_small;
			record.hitted_entity = this;
			record.location = ray(t_small);
			record.hitted_entity_material = material;
			record.surface_normal = Whitted::normalize(record.location - m_center);

			return record;
		}

	private:
		WhittedMaterial* material;
		glm::vec3 m_center;
		float m_radius;
		float radius_squared;	// to reduce computation when calculating intersections
	};
}

#endif // !SPHERE_H
