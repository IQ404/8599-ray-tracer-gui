/*****************************************************************//**
 * \file   Ray.h
 * \brief  Data structure representing the ray
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>
#include "VectorFloat.h"

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

namespace AccelerationStructure
{
	struct Ray
	{
		Ray(const glm::vec3& origin, const glm::vec3& direction)
			: m_origin(origin), m_direction(direction)
		{
			t_min = 0.0;
			t_max = std::numeric_limits<double>::max();

			direction_reciprocal = glm::vec3{ 1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z };
		}

		glm::vec3 operator()(double t) const
		{
			return m_origin + (float)t * m_direction;
		}

		glm::vec3 m_origin;
		glm::vec3 m_direction;
		glm::vec3 direction_reciprocal;
		double t_min;
		double t_max;
	};
}

#endif // !RAY_H