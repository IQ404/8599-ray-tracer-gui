/*****************************************************************//**
 * \file   BVH.h
 * \brief  The implementation of the Bounding Volume Hierarchy data structure
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef BVH_H
#define BVH_H

#include <vector>
#include <algorithm>

#include "BoundingVolume.h"
#include "Entity.h"
#include "IntersectionRecord.h"

namespace AccelerationStructure
{
	class BVH_Node
	{
	public:

		BVH_Node()
		{

		}

		~BVH_Node()
		{
			delete left;
			delete right;
			// we are not deleting the entities in the tree
		}

		// Data members:
		BVH_Node* left = nullptr;
		BVH_Node* right = nullptr;
		AABB_3D bounding_volume = AABB_3D();
		Whitted::Entity* entity = nullptr;
		int dividing_axis = 0;
		int number_of_primitives = 0;
	};

	class BVH
	{
	public:
		enum class DividingMethod
		{
			Median,
			Surface_Area_Heuristic
		};

		BVH(
			std::vector<Whitted::Entity*> primitives,
			int max_number_of_primitives_in_one_node = 1,
			DividingMethod dividing_method = DividingMethod::Median
		)
			:
			m_dividing_method(dividing_method),
			m_primitives(std::move(primitives)),
			m_max_number_of_primitives_in_one_node(std::min(255, max_number_of_primitives_in_one_node))
		{
			if (m_primitives.empty())
			{
				return;
			}
			root = build_BVH(m_primitives);
		}

		~BVH()
		{
			delete root;
		}

		Whitted::IntersectionRecord traverse_BVH_from_root(const Ray& ray) const
		{
			if (!root)	// no primitives at all in the scene
			{
				return {};	// no intersection
			}

			return traverse_BVH_from_node(root, ray);
		}

		Whitted::IntersectionRecord traverse_BVH_from_node(BVH_Node* node, const Ray& ray) const
		{
			std::array<int, 3> ray_direction_is_negative{ray.m_direction.x < 0.0f, ray.m_direction.y < 0.0f, ray.m_direction.z < 0.0f};
			
			if (!node->bounding_volume.intersects_with_ray(ray, ray.direction_reciprocal, ray_direction_is_negative))
			{
				// case: no intersection:
				return {};
			}
			// case: has intersection with the box:
			if ((node->left == nullptr) && (node->right == nullptr))	// case: we are at leaf node
			{
				return node->entity->GetIntersectionRecord(ray);
			}
			// we are not at leaf node:
			Whitted::IntersectionRecord left_part = traverse_BVH_from_node(node->left, ray);
			Whitted::IntersectionRecord right_part = traverse_BVH_from_node(node->left, ray);

			return (left_part.t > right_part.t) ? (left_part) : (right_part);
		}

	public:		// public data members:
		BVH_Node* root = nullptr;

	private:
		BVH_Node* build_BVH(std::vector<Whitted::Entity*> entities)
		// Assume entities.size() != 0
		{
			/*
			TODO:
			stop when each node has less than 5 primitives (right now we stop at 1)
			*/

			BVH_Node* local_root = new BVH_Node();
			if (entities.size() == 1)	// leaf node
			{
				local_root->left = nullptr;
				local_root->right = nullptr;
				local_root->entity = entities[0];
				local_root->bounding_volume = entities[0]->Get3DAABB();

				return local_root;
			}
			else if (entities.size() == 2)
			{
				local_root->left = build_BVH(std::vector<Whitted::Entity*>{entities[0]});
				local_root->right = build_BVH(std::vector<Whitted::Entity*>{entities[1]});
				// local_root->entity remains nullptr
				local_root->bounding_volume = local_root->left->bounding_volume.Union_with_3D_AABB(local_root->right->bounding_volume);

				return local_root;
			}
			else
			{
				AABB_3D AABB_of_all_centroids;	// contains nothing at this point
				for (int i = 0; i < entities.size(); i++)
				{
					AABB_of_all_centroids = AABB_of_all_centroids.Union_with_point(entities[i]->Get3DAABB().center_vector());
					// We use AABB of centroids instead of entities' own AABBs because we want partitions according to
					// entities' number, and thus we don't care about the shape of each entity.
				}
				switch (AABB_of_all_centroids.longest_axis())
				{
				// sort from small to large (see https://cplusplus.com/reference/algorithm/sort/)
				case X_axis:
					std::sort(entities.begin(), entities.end(),
						[](Whitted::Entity* a, Whitted::Entity* b)
						{
							return (a->Get3DAABB().center_vector().x < b->Get3DAABB().center_vector().x);
						}
					);
					break;
				case Y_axis:
					std::sort(entities.begin(), entities.end(),
						[](Whitted::Entity* a, Whitted::Entity* b)
						{
							return (a->Get3DAABB().center_vector().y < b->Get3DAABB().center_vector().y);
						}
					);
					break;
				case Z_axis:
					std::sort(entities.begin(), entities.end(),
						[](Whitted::Entity* a, Whitted::Entity* b)
						{
							return (a->Get3DAABB().center_vector().z < b->Get3DAABB().center_vector().z);
						}
					);
					break;
				}

				auto iterator_head = entities.begin();
				auto iterator_median = entities.begin() + (entities.size() / 2);
				auto iterator_tail = entities.end();

				std::vector<Whitted::Entity*> left_half = std::vector<Whitted::Entity*>(iterator_head, iterator_median);
				std::vector<Whitted::Entity*> right_half = std::vector<Whitted::Entity*>(iterator_median, iterator_tail);

				local_root->left = build_BVH(left_half);
				local_root->right = build_BVH(right_half);

				local_root->bounding_volume = local_root->left->bounding_volume.Union_with_3D_AABB(local_root->right->bounding_volume);

				return local_root;
			}
		}

		// private data members:
		const DividingMethod m_dividing_method;
		const int m_max_number_of_primitives_in_one_node;
		std::vector<Whitted::Entity*> m_primitives;
	};

}

#endif // !BVH_H