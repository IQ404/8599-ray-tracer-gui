/*****************************************************************//**
 * \file   VectorFloat.h
 * \brief  Auxiliary functionalities for glm vectors used in Whitted Style ray tracing
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef VECTORFLOAT_H
#define VECTORFLOAT_H

#include <glm/glm.hpp>		// to use glm vec
#include <cmath>

namespace Whitted
{
	inline glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, const float& t)
	{
		return a * (1 - t) + b * t;
	}

	inline glm::vec3 normalize(const glm::vec3& v)		// return v if v is zero vector
	{
		float length_squared = v.x * v.x + v.y * v.y + v.z * v.z;
		if (length_squared > 0)
		{
			float inverse_length = 1 / std::sqrtf(length_squared);
			return glm::vec3{v.x * inverse_length, v.y * inverse_length, v.z * inverse_length};
		}
		return v;
	}
}

#endif // !VECTORFLOAT_H