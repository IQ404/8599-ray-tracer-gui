/*****************************************************************//**
 * \file   BoundingVolume.h
 * \brief  The representations of bounding boxes
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef BOUNDINGVOLUME_H
#define BOUNDINGVOLUME_H

#include <limits>
#include <array>

#include "Ray.h"

namespace AccelerationStructure
{
	enum Axis
	{
		X_axis,
		Y_axis,
		Z_axis
	};

	class AABB_3D
	{
	public:

		// Constructors:

		AABB_3D()	// AABB that contains nothing
		{
			constexpr double infinity = std::numeric_limits<double>::max();
			constexpr double minus_infinity = std::numeric_limits<double>::lowest();
			min_slab_values = glm::vec3{ infinity,infinity,infinity };
			max_slab_values = glm::vec3{ minus_infinity,minus_infinity,minus_infinity };
			// Note that this will set the components as inf / -inf
		}

		AABB_3D(const glm::vec3& point)		// AABB that contains only a point
			: min_slab_values{ point }, max_slab_values{ point }
		{

		}

		AABB_3D(const glm::vec3& point_1, const glm::vec3& point_2)		// AABB that takes point_1 - point_2 as (secondary) diagonal
		{
			min_slab_values = glm::vec3{ std::fmin(point_1.x, point_2.x), std::fmin(point_1.y, point_2.y), std::fmin(point_1.z, point_2.z) };
			max_slab_values = glm::vec3{ std::fmax(point_1.x, point_2.x), std::fmax(point_1.y, point_2.y), std::fmax(point_1.z, point_2.z) };
		}

		// Methods:

		bool contains_point(const glm::vec3& point)
		{
			return (
				(min_slab_values.x <= point.x) && (point.x <= max_slab_values.x)
				&&
				(min_slab_values.y <= point.y) && (point.y <= max_slab_values.y)
				&&
				(min_slab_values.z <= point.z) && (point.z <= max_slab_values.z)
				);
			// Note how this IS consistent with AABB_3D() and AABB_3D(const glm::vec3& point)
			// return true for inf large box versus a point that is infinitely far
		}

		bool intersects_with_3D_AABB(const AABB_3D& box)
		{
			return (
				(max_slab_values.x >= box.min_slab_values.x) && (min_slab_values.x <= box.max_slab_values.x)
				&&
				(max_slab_values.y >= box.min_slab_values.y) && (min_slab_values.y <= box.max_slab_values.y)
				&&
				(max_slab_values.z >= box.min_slab_values.z) && (min_slab_values.z <= box.max_slab_values.z)
				);
			// Note that this return true when evaluating box of everything (inf large box) versus anything
			// and return false when evaluating box of nothing versus anything except box of everything
		}

		AABB_3D Intersection_with_3D_AABB(const AABB_3D& box)
		// This should be used logically AFTER bool intersects(const AABB_3D& box) returns true. Or the result may not be correct!
		{
			return AABB_3D
			{
				glm::vec3{std::fmin(max_slab_values.x, box.max_slab_values.x), std::fmin(max_slab_values.y, box.max_slab_values.y), std::fmin(max_slab_values.z, box.max_slab_values.z)}
				,
				glm::vec3{std::fmax(min_slab_values.x, box.min_slab_values.x), std::fmax(min_slab_values.y, box.min_slab_values.y), std::fmax(min_slab_values.z, box.min_slab_values.z)}
			};
			// return box of everything (inf large box) when evaluating box of nothing versus box of everything
		}

		glm::vec3 scaled_by_the_box(const glm::vec3& point) const
		{
			glm::vec3 relative_vector = point - min_slab_values;

			if (max_slab_values.x > min_slab_values.x)
			{
				relative_vector.x /= max_slab_values.x - min_slab_values.x;
			}
			if (max_slab_values.y > min_slab_values.y)
			{
				relative_vector.y /= max_slab_values.y - min_slab_values.y;
			}
			if (max_slab_values.z > min_slab_values.z)
			{
				relative_vector.z /= max_slab_values.z - min_slab_values.z;
			}

			return relative_vector;
			
			// directly return the unscaled relative vector if the box is a point
			// return {-inf,-inf,-inf} if the box contains nothing
		}

		glm::vec3 center_vector() const
		{
			return 0.5f * (max_slab_values + min_slab_values);
		}

		glm::vec3 diagonal_vector() const
		{
			return max_slab_values - min_slab_values;
		}

		double total_area() const
		{
			glm::vec3 diagonal = diagonal_vector();
			return (diagonal.x * diagonal.y + diagonal.y * diagonal.z + diagonal.x * diagonal.z) * 2;
		}

		int longest_axis() const
		{
			glm::vec3 diagonal = diagonal_vector();

			if ((diagonal.x > diagonal.y) && (diagonal.x > diagonal.z))
			{
				return X_axis;
			}
			else if (diagonal.y > diagonal.z)
			{
				return Y_axis;
			}
			else
			{
				return Z_axis;
			}
		}

		const glm::vec3& operator[](int i) const
		{
			return (i == 0) ? (min_slab_values) : (max_slab_values);
		}

		AABB_3D Union_with_point(const glm::vec3& point)
		{
			AABB_3D union_box;
			union_box.min_slab_values = glm::min(min_slab_values, point);
			union_box.max_slab_values = glm::max(max_slab_values, point);
			return union_box;
			// Note that such union will never result in a shrinked box
		}

		AABB_3D Union_with_3D_AABB(const AABB_3D& box)
		{
			AABB_3D union_box;
			union_box.min_slab_values = glm::min(min_slab_values, box.min_slab_values);
			union_box.max_slab_values = glm::max(max_slab_values, box.max_slab_values);
			return union_box;
			// Note that glm::min and glm::max conduct componentwise comparison
		}

		bool intersects_with_ray(
			const Ray& ray, 
			const glm::vec3& ray_direction_reciprocal, // multiplication is (slightly) faster than division
			const std::array<int,3>& ray_direction_is_negative
		)
		// We assume there is no perfectly axis-aligned ray, so that the intersection point for each slab exists and is unique.
		{
			// First assume all the three components of the ray direction are positive (we will correct this later), we thus have:
			//glm::vec3 t_in = (min_slab_values - ray.m_origin) * ray_direction_reciprocal;
			//glm::vec3 t_out = (max_slab_values - ray.m_origin) * ray_direction_reciprocal;

			//// Correction according to the componentwise ray direction:
			//if (ray_direction_is_negative[0])
			//{
			//	std::swap(t_in.x, t_out.x);
			//}
			//if (ray_direction_is_negative[1])
			//{
			//	std::swap(t_in.y, t_out.y);
			//}
			//if (ray_direction_is_negative[2])
			//{
			//	std::swap(t_in.z, t_out.z);
			//}

			//float last_in = std::max(t_in.x, std::max(t_in.y, t_in.z));
			//float first_out = std::max(t_out.x, std::max(t_out.y, t_out.z));

			//return ((last_in < first_out) && (first_out >= 0.0f));
			//________________________________________________________________________________
			float t_Min_x = (min_slab_values.x - ray.m_origin.x) * ray_direction_reciprocal.x;
			float t_Min_y = (min_slab_values.y - ray.m_origin.y) * ray_direction_reciprocal.y;
			float t_Min_z = (min_slab_values.z - ray.m_origin.z) * ray_direction_reciprocal.z;
			float t_Max_x = (max_slab_values.x - ray.m_origin.x) * ray_direction_reciprocal.x;
			float t_Max_y = (max_slab_values.y - ray.m_origin.y) * ray_direction_reciprocal.y;
			float t_Max_z = (max_slab_values.z - ray.m_origin.z) * ray_direction_reciprocal.z;

			if (ray_direction_is_negative[0])
			{
				float temp = t_Min_x;
				t_Min_x = t_Max_x;
				t_Max_x = temp;
			}

			if (ray_direction_is_negative[1])
			{
				float temp = t_Min_y;
				t_Min_y = t_Max_y;
				t_Max_y = temp;
			}

			if (ray_direction_is_negative[2])
			{
				float temp = t_Min_z;
				t_Min_z = t_Max_z;
				t_Max_z = temp;
			}

			float tEnter = std::max(t_Min_x, std::max(t_Min_y, t_Min_z));
			float tExit = std::min(t_Max_x, std::min(t_Max_y, t_Max_z));

			if (tExit >= 0 && tEnter <= tExit)
				return true;

			return false;
		}

		/*
		C++ side-note:
		std::array<T,size_t> is used as "a C-style array that will NOT (automatically) decay".
		*/

	public:		// Data members:
		glm::vec3 min_slab_values;
		glm::vec3 max_slab_values;
	};
}

#endif // !BOUNDINGVOLUME_H