/*****************************************************************//**
 * \file   IntersectionRecord.h
 * \brief  A structure to store the information of a ray intersection
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef INTERSECTIONRECORD_H
#define INTERSECTIONRECORD_H

#include "WhittedMaterial.h"

namespace Whitted
{
	class Entity;

	class IntersectionRecord
	{
	public:
		IntersectionRecord()
		{
			has_intersection = false;
			t = std::numeric_limits<double>::max();
			location = glm::vec3{ 0.0f, 0.0f, 0.0f };
			surface_normal = glm::vec3{ 0.0f, 0.0f, 0.0f };
			hitted_entity = nullptr;
			hitted_entity_material = nullptr;
		}

		bool has_intersection;
		double t;	// Don't forget to normalize the corresponding ray_direction
		glm::vec3 location;
		glm::vec3 surface_normal;
		
		Entity* hitted_entity;
		WhittedMaterial* hitted_entity_material;
	};
}

#endif // !INTERSECTIONRECORD_H
