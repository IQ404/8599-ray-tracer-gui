/*****************************************************************//**
 * \file   NPPTToolbox.h
 * \brief  The Non-Physical Path Tracing Toolbox
 * 
 * \author Xiaoyang Liu
 * \date   July 2023
 *********************************************************************/

#ifndef NPPTTOOLBOX_H
#define NPPTTOOLBOX_H

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>
//#include <cassert>
#include <iostream>

#include "Walnut/Random.h"

namespace NP_PathTracing
{
	// Global Enums:
	
	enum DiffuseModel
	{
		IN_Sphere,
		ON_Sphere,
		IN_Hemisphere
	};

	// Protected Global Variables:

	inline DiffuseModel& GetActiveDiffuseModel()
	{
		static DiffuseModel diffuse_model = ON_Sphere;
		return diffuse_model;
	}

	inline int& GetGamma()
	{
		static int gamma = 1;
		return gamma;
	}

	// Numeric Constants:

	constexpr float positive_infinity = std::numeric_limits<float>::infinity();	// (std::numeric_limits<float>::max() < std::numeric_limits<float>::infinity()) == true
	constexpr float pi = 3.1415926'535897932385;	// ??? Is the precision of a float able to store the digits after 3.1415926?

	constexpr float ray_starting_offset = 0.001f;	// for Shadow Acne Elimination
	constexpr int max_bounce_depth = 50;

	// Utility Functions:

	inline int round_real_to_int(float r)
	{
		if (r >= 0.0)
		{
			if (r - int(r) >= 0.5)
			{
				return int(r) + 1;
			}
			return int(r);
		}
		if (int(r) - r >= 0.5)
		{
			return int(r) - 1;
		}
		return int(r);
	}

	inline float degrees_to_radians(float degrees)
	{
		return (degrees / 180.0f) * pi;
	}

	inline float random_real_number(float min, float max)
	{
		//assert(min <= max);
		return min + (max - min) * Walnut::Random::Float();
	}

	inline float fast_random()
		/*
		Implementation: PCG
		References see:
			https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
			https://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
		*/
	{
		// TODO: currently not implemented because it will generate the same image for the same scene.
	}

	inline bool near_zero(glm::vec3 v)
	{
		static constexpr float minimum = 1e-8;
		return ((std::fabs(v[0]) < minimum) && (std::fabs(v[1]) < minimum) && (std::fabs(v[2]) < minimum));
		// C++ side note: fabs() is the floating point number version of abs()
	}

	inline std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
	{
		return os << v.x << ' ' << v.y << ' ' << v.z;
	}

	inline glm::vec3 random_in_unit_sphere()
	{
		while (true)
		{
			glm::vec3 p = Walnut::Random::Vec3(-1, 1);
			if (glm::dot(p,p) >= 1.0)
			{
				continue;
			}
			return p;
		}
	}

	inline glm::vec3 random_unit_vector()
	{
		// TODO: check the vector returned by random_in_unit_sphere is non-zero 
		return glm::normalize(random_in_unit_sphere());
	}

	inline glm::vec3 random_in_unit_hemisphere(const glm::vec3& normal)
	{
		// Note that this implementation is a hack (not mathmatically accurate)
		glm::vec3 p = random_in_unit_sphere();
		if (glm::dot(p, normal) >= 0.0)
		{
			return p;
		}
		return -p;
	}

	inline glm::vec3 random_in_unit_xy_disk()
	{
		while (true)
		{
			glm::vec3 p{ random_real_number(-1.0,1.0),random_real_number(-1.0,1.0),0.0 };
			if (glm::dot(p,p) >= 1.0)
			{
				continue;
			}
			return p;
		}
	}

	inline glm::vec3 direction_of_mirror_reflection(const glm::vec3& incident_direction, const glm::vec3& normal)
	{
		return incident_direction - 2.0f * glm::dot(incident_direction, normal) * normal;
	}

	inline glm::vec3 direction_of_Snell_refraction(const glm::vec3& unit_incident_direction, const glm::vec3& unit_incident_normal, float etaIN_over_etaOUT)
	{
		glm::vec3 refraction_direction_tangential = etaIN_over_etaOUT * (unit_incident_direction + std::fmin(glm::dot(-unit_incident_direction, unit_incident_normal), 1.0f) * unit_incident_normal);
		glm::vec3 refraction_direction_normal = -std::sqrt(std::fabs(1.0f - glm::dot(refraction_direction_tangential, refraction_direction_tangential))) * unit_incident_normal;
		// Note: here std::fmin and std::fabs are to eliminate computation errors caused by floating point precision.
		return refraction_direction_tangential + refraction_direction_normal;
	}

}

// For better code readability (as 3D vectors will represent things with different physical meanings):
// ??? Should we put these inside the NP_PathTracing namespace?
using Point3D = glm::vec3;
using ColorRGB = glm::vec3;

#endif // !NPPTTOOLBOX_H