/*****************************************************************//**
 * \file   TriangleMesh.h
 * \brief  Representation of a mesh consisting of triangle-primitives in a Whitted-Style ray-traced scene
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "Entity.h"

namespace Whitted
{
	inline bool RayTriangleIntersection(
		const glm::vec3& vertice_1, 
		const glm::vec3& vertice_2, 
		const glm::vec3& vertice_3,
		const glm::vec3& ray_origin,
		const glm::vec3& ray_direction,
		float& t_intersection,
		float& barycentric_coordinate_2,
		float& barycentric_coordinate_3
		// barycentric_coordinate_1 = 1 - barycentric_coordinate_2 - barycentric_coordinate_3
		)
	// implemented using Moller-Trumbore algorithm
	{
		glm::vec3 E_1 = vertice_2 - vertice_1;
		glm::vec3 E_2 = vertice_3 - vertice_1;
		glm::vec3 S = ray_origin - vertice_1;
		glm::vec3 S_1 = glm::cross(ray_direction, E_2);
		glm::vec3 S_2 = glm::cross(S, E_1);

		float denominator = glm::dot(S_1, E_1);

		t_intersection = glm::dot(S_2, E_2) / denominator;
		barycentric_coordinate_2 = glm::dot(S_1, S) / denominator;
		barycentric_coordinate_3 = glm::dot(S_2, ray_direction) / denominator;

		return ((t_intersection > 0.0f) && 
			(barycentric_coordinate_2 > 0.0f) && 
			(barycentric_coordinate_3 > 0.0f) && 
			((1 - barycentric_coordinate_2 - barycentric_coordinate_3) > 0.0f));	// TODO: add epsilon to account for floating point precision error?
	}

	class TriangleMesh : public Entity
	{
	public:
		TriangleMesh
		(
			const glm::vec3* vertices,
			const uint32_t* vertices_indices,
			const uint32_t& number_of_triangles,
			const glm::vec2* texture_coordinates
		)
			: m_number_of_triangles{ number_of_triangles }
		{
			uint32_t number_of_vertices = 0;
			for (uint32_t i = 0; i < (number_of_triangles * 3); i++)
			{
				if (vertices_indices[i] > number_of_vertices)
				{
					number_of_vertices = vertices_indices[i];
				}
			}
			number_of_vertices += 1;

			m_vertices = std::unique_ptr<glm::vec3[]>(new glm::vec3[number_of_vertices]);
			std::memcpy(m_vertices.get(), vertices, sizeof(glm::vec3) * number_of_vertices);
			// Note that vertices and texture coordinates maps bijectively
			m_texture_coordinates = std::unique_ptr<glm::vec2[]>(new glm::vec2[number_of_vertices]);
			std::memcpy(m_texture_coordinates.get(), texture_coordinates, sizeof(glm::vec2) * number_of_vertices);

			m_vertices_indices = std::unique_ptr<uint32_t[]>(new uint32_t[number_of_triangles * 3]);
			std::memcpy(m_vertices_indices.get(), vertices_indices, sizeof(uint32_t) * number_of_triangles * 3);
		}

		virtual glm::vec3 GetDiffuseColor(const glm::vec2& texture_coordinates) const override
		{
			// A procedural texture algorithm generating chessboard-like pattern for the floor
			float frequency = 5;
			float pattern = (std::fmodf(texture_coordinates.x * frequency, 1) > 0.5) ^ (std::fmodf(texture_coordinates.y * frequency, 1) > 0.5);
			// See https://learn.microsoft.com/en-us/cpp/cpp/bitwise-exclusive-or-operator-hat?view=msvc-170
			return Whitted::lerp(glm::vec3(0.815, 0.235, 0.031), glm::vec3(0.937, 0.937, 0.231), pattern);
		}

		virtual bool Intersect(
			const glm::vec3& light_origin,
			const glm::vec3& light_direction,
			float& closerT,
			uint32_t& triangle_index,
			glm::vec2& barycentric_coordinates
		) const override
		{
			bool intersect = false;
			for (uint32_t index = 0; index < m_number_of_triangles; index++)
			{
				const glm::vec3& vertice_1 = m_vertices[m_vertices_indices[index * 3]];
				const glm::vec3& vertice_2 = m_vertices[m_vertices_indices[index * 3 + 1]];
				const glm::vec3& vertice_3 = m_vertices[m_vertices_indices[index * 3 + 2]];
				float t_current;
				float barycentric_coordinate_2;
				float barycentric_coordinate_3;
				if (Whitted::RayTriangleIntersection(vertice_1, vertice_2, vertice_3, light_origin, light_direction, t_current, barycentric_coordinate_2, barycentric_coordinate_3)
					&&
					(t_current < closerT)
					)
				{
					closerT = t_current;
					barycentric_coordinates.x = barycentric_coordinate_2;
					barycentric_coordinates.y = barycentric_coordinate_3;
					triangle_index = index;
					intersect |= true;
				}
			}
			return intersect;
		}

		virtual void GetHitInfo(
			const glm::vec3&,
			const glm::vec3&,
			const uint32_t& triangle_index,
			const glm::vec2& barycentric_coordinates,
			glm::vec3& surface_normal,
			glm::vec2& texture_coordinates
		) const override
		{
			const glm::vec3& vertice_1 = m_vertices[m_vertices_indices[triangle_index * 3]];
			const glm::vec3& vertice_2 = m_vertices[m_vertices_indices[triangle_index * 3 + 1]];
			const glm::vec3& vertice_3 = m_vertices[m_vertices_indices[triangle_index * 3 + 2]];
			// Note that triangle_index starts from 0
			surface_normal = Whitted::normalize(
				glm::cross(
					Whitted::normalize(vertice_2 - vertice_1), Whitted::normalize(vertice_3 - vertice_2)
				)
			);
			// Note: we computed this normal with the orientation that is according to the right-hand rule 
			// because we want to find the "outward" direction.

			const glm::vec2& texture_coordinates_vertice_1 = m_texture_coordinates[m_vertices_indices[triangle_index * 3]];
			const glm::vec2& texture_coordinates_vertice_2 = m_texture_coordinates[m_vertices_indices[triangle_index * 3 + 1]];
			const glm::vec2& texture_coordinates_vertice_3 = m_texture_coordinates[m_vertices_indices[triangle_index * 3 + 2]];

			texture_coordinates =
				(1 - barycentric_coordinates.x - barycentric_coordinates.y) * texture_coordinates_vertice_1 +
				barycentric_coordinates.x * texture_coordinates_vertice_2 +
				barycentric_coordinates.y * texture_coordinates_vertice_3;
		}

	private:
		uint32_t m_number_of_triangles;
		std::unique_ptr<glm::vec3[]> m_vertices;
		std::unique_ptr<glm::vec2[]> m_texture_coordinates;
		std::unique_ptr<uint32_t[]> m_vertices_indices;
	};
}

#endif // !TRIANGLEMESH_H
