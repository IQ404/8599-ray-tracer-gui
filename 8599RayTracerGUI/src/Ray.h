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
#include "NPPTToolbox.h"

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

namespace NP_PathTracing
{
	class Ray
	{
	public:

		// Constructors:

		Ray()	// both origin and direction are initialized to (0,0,0), and thus the line degenerates to the point (0,0,0).
		{

		}

		Ray(const Point3D& origin, const glm::vec3& direction)
			: orig{ origin }, dir{ direction }
		{

		}

		// Getters:

		Point3D origin() const
		{
			return orig;
		}

		glm::vec3 direction() const
		{
			return dir;
		}

		Point3D at(float t) const		// The point P(t) = A + tB, where A is the origin of the line and B is the direction of the line.
		{
			return orig + t * dir;
		}

	private:

		// Our ray is encoded as the R->R^3 function P(t) = A + tB, where A is the origin of the line and B is the direction of the line.

		Point3D orig{ 0.0f,0.0f,0.0f };
		glm::vec3 dir{ 0.0f, 0.0f, 0.0f };
	};
}

#endif // !RAY_H