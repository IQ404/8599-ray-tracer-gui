/*****************************************************************//**
 * \file   HitRecord.h
 * \brief  The data structure used to (temporarily) store relevant information when a ray hits on an entity
 * 
 * \author Xiaoyang Liu
 * \date   July 2023
 *********************************************************************/

#ifndef HITRECORD_H
#define HITRECORD_H

#include "Ray.h"

namespace NP_PathTracing
{
	class Material;		// this forward declares the class NP_PathTracing::Material

	struct HitRecord
	{
		std::shared_ptr<Material> material_pointer;
		Point3D point{ 0.0f,0.0f,0.0f };
		glm::vec3 normal{0.0f, 0.0f, 0.0f};
		float t;
		bool is_hitting_front_face;

		// Assume for any geometric entity rendered by this ray tracer, there is a defined front face (and thus a back face) and thus a front (i.e. outward) normal. (??? What about Klein bottle/ring?)
		// We choose to set the normal recorded in HitRecord to always towards the ray:
		void set_normal(const Ray& ray, const glm::vec3& outward_normal)		// Setting the normal and the front-back bool
		{
			is_hitting_front_face = (glm::dot(ray.direction(), outward_normal) <= 0.0);	// Note: Hitting at right angle is counted as hitting from outside
			normal = is_hitting_front_face ? (outward_normal) : (-outward_normal);
		}
	};
}

#endif // !HITRECORD_H