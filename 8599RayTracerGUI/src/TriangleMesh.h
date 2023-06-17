/*****************************************************************//**
 * \file   TriangleMesh.h
 * \brief  Representation of a mesh consisting of triangle-primitives in a Whitted-Style ray-traced scene
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include <cassert>

#include "OBJ_Loader.h"
#include "BVH.h"

namespace Whitted
{
	inline bool RayTriangleIntersection(
		const glm::vec3& vertice_1, 
		const glm::vec3& vertice_2, 
		const glm::vec3& vertice_3,
		const glm::vec3& ray_origin,
		const glm::vec3& ray_direction,
		double& t_intersection
		//float& barycentric_coordinate_2,
		//float& barycentric_coordinate_3
		// Note: barycentric_coordinate_1 = 1 - barycentric_coordinate_2 - barycentric_coordinate_3
		)
	// implemented using Moller-Trumbore algorithm (TODO: get a pen and a paper to derive the math behind this algorithm!!!)
	{
		glm::vec3 E_1 = vertice_2 - vertice_1;
		glm::vec3 E_2 = vertice_3 - vertice_1;
		glm::vec3 S = ray_origin - vertice_1;
		glm::vec3 S_1 = glm::cross(ray_direction, E_2);
		glm::vec3 S_2 = glm::cross(S, E_1);

		double inverse_denominator = 1.0 / glm::dot(S_1, E_1);

		t_intersection = glm::dot(S_2, E_2) * inverse_denominator;
		double barycentric_coordinate_2 = glm::dot(S_1, S) * inverse_denominator;
		double barycentric_coordinate_3 = glm::dot(S_2, ray_direction) * inverse_denominator;

		return ((t_intersection > 0.0) && 
			(barycentric_coordinate_2 > 0.0) && 
			(barycentric_coordinate_3 > 0.0) && 
			((1.0 - barycentric_coordinate_2 - barycentric_coordinate_3) > 0.0));	// TODO: add epsilon to account for floating point precision error?
	}

	class TrianglePrimitive : public Entity
	/*
	Assumptions:
	The vertices are declared in anti-clockwise order.
	*/
	{
	public:
		TrianglePrimitive(glm::vec3 _vertice_a, glm::vec3 _vertice_b, glm::vec3 _vertice_c, WhittedMaterial* _material = nullptr)
			: vertice_a(_vertice_a), vertice_b(_vertice_b), vertice_c(_vertice_c), material(_material)
		{
			m_surface_normal = Whitted::normalize(glm::cross(_vertice_b - _vertice_a, _vertice_c - _vertice_a));
		}

		// Methods:

		virtual AccelerationStructure::AABB_3D Get3DAABB() override
		{
			return (AccelerationStructure::AABB_3D{vertice_a, vertice_b}).Union_with_point(vertice_c);
		}

		virtual glm::vec3 GetDiffuseColor(const glm::vec2&) const override
		{
			return glm::vec3{0.5f, 0.5f, 0.5f};		// TODO: whenever possible, use material information instead?
		}

		virtual void GetHitInfo(
			const glm::vec3& intersection,
			const glm::vec3& light_direction,
			const uint32_t& triangle_index,
			const glm::vec2& barycentric_coordinates,
			glm::vec3& surface_normal,
			glm::vec2& texture_coordinates
		) const override
		{
			surface_normal = m_surface_normal;
		}

		virtual IntersectionRecord GetIntersectionRecord(AccelerationStructure::Ray ray) override
		{
			IntersectionRecord record;
			if (RayTriangleIntersection(vertice_a, vertice_b, vertice_c, ray.m_origin, ray.m_direction, record.t))
			{
				record.has_intersection = true;
				record.hitted_entity_material = material;
				record.hitted_entity = this;
				record.surface_normal = m_surface_normal;
				record.location = ray(record.t);	// Note that this is calling the operator()
			}
			else
			{
				record.t = std::numeric_limits<double>::max();
			}
			return record;
		}

	public:		// Data members:
		glm::vec3 vertice_a;
		glm::vec3 vertice_b;
		glm::vec3 vertice_c;
		glm::vec3 m_surface_normal;
		WhittedMaterial* material;
	};

	class TriangleMesh : public Entity
	{
	public:
		TriangleMesh(const std::string& file_path)
		{
			constexpr float mesh_scale = 1.0f;

			objl::Loader Robert_Smith_Loader;
			Robert_Smith_Loader.LoadFile(file_path);
			// assume only one mesh is loaded...

			assert(Robert_Smith_Loader.LoadedMeshes.size() == 1);

			objl::Mesh loaded_mesh = Robert_Smith_Loader.LoadedMeshes[0];
			
			glm::vec3 mesh_range_min = glm::vec3{ std::numeric_limits<float>::infinity(),std::numeric_limits<float>::infinity(),std::numeric_limits<float>::infinity() };
			glm::vec3 mesh_range_max = glm::vec3{ -std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity() };

			for (int i = 0; i < loaded_mesh.Vertices.size(); i += 3)	// one loop for each triangle
			{
				std::array<glm::vec3, 3> current_triangle;
				for (int j = 0; j < 3; j++)
				{
					glm::vec3 scaled_vertice = mesh_scale * glm::vec3{loaded_mesh.Vertices[i + j].Position.X, loaded_mesh.Vertices[i + j].Position.Y, loaded_mesh.Vertices[i + j].Position.Z };
					current_triangle[j] = scaled_vertice;

					mesh_range_min = glm::vec3{ std::min(mesh_range_min.x, scaled_vertice.x), std::min(mesh_range_min.y, scaled_vertice.y), std::min(mesh_range_min.z, scaled_vertice.z) };
					mesh_range_max = glm::vec3{ std::max(mesh_range_max.x, scaled_vertice.x), std::max(mesh_range_max.y, scaled_vertice.y), std::max(mesh_range_max.z, scaled_vertice.z) };
				}
				// Note that we want each triangle to have its own material:
				WhittedMaterial* unified_material = new WhittedMaterial{ Diffuse_Glossy, glm::vec3{0.5f,0.5f,0.5f}, glm::vec3{0.0f,0.0f,0.0f} };
				unified_material->phong_diffuse = 0.6f;
				unified_material->phong_specular = 0.0f;
				unified_material->specular_size_factor = 0.0f;
				triangle_primitives.emplace_back(current_triangle[0], current_triangle[1], current_triangle[2], unified_material);	// implicitly calls TrianglePrimitive's constructor
			}
			bounding_AABB = AccelerationStructure::AABB_3D{ mesh_range_min,mesh_range_max };
			std::vector<Entity*> entity_pointers;
			for (TrianglePrimitive& triangle : triangle_primitives)
			{
				entity_pointers.push_back(&triangle);
			}
			bvh = new AccelerationStructure::BVH{ entity_pointers };
		}

		virtual AccelerationStructure::AABB_3D Get3DAABB() override
		{
			return bounding_AABB;
		}

		virtual IntersectionRecord GetIntersectionRecord(AccelerationStructure::Ray ray) override
		{
			IntersectionRecord record;
			if (bvh)
			{
				record = bvh->traverse_BVH_from_root(ray);
			}
			return record;
		}

		virtual glm::vec3 GetDiffuseColor(const glm::vec2& texture_coordinates) const override
		{
			// A procedural texture algorithm generating chessboard-like pattern for the floor
			float frequency = 5;
			float pattern = (std::fmodf(texture_coordinates.x * frequency, 1) > 0.5) ^ (std::fmodf(texture_coordinates.y * frequency, 1) > 0.5);
			// See https://learn.microsoft.com/en-us/cpp/cpp/bitwise-exclusive-or-operator-hat?view=msvc-170
			return Whitted::lerp(glm::vec3(0.815, 0.235, 0.031), glm::vec3(0.937, 0.937, 0.231), pattern);
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
		std::vector<TrianglePrimitive> triangle_primitives;
		std::unique_ptr<glm::vec3[]> m_vertices;
		std::unique_ptr<glm::vec2[]> m_texture_coordinates;
		std::unique_ptr<uint32_t[]> m_vertices_indices;
		AccelerationStructure::AABB_3D bounding_AABB;
		AccelerationStructure::BVH* bvh;
	};
}

#endif // !TRIANGLEMESH_H
