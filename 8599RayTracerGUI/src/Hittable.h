/*****************************************************************//**
 * \file   Hittable.h
 * \brief  The representation of things that can be hitted by the ray in the 8599 non-physical path tracer
 * 
 * \author Xiaoyang Liu
 * \date   July 2023
 *********************************************************************/

#ifndef HITTABLE_H
#define HITTABLE_H

#include <memory>	// to use smart pointers
#include <vector>
#include "HitRecord.h"

namespace NP_PathTracing
{

	class Hittable
	{
	public:
		virtual bool is_hit_by(const Ray& ray, float t_min, float t_max, HitRecord& record) const = 0;
	};

	/*
	C++ side notes on shared pointers:

	std::shared_ptr<T> is a pointer encapsulated with reference-counting semantics such that the pointed object is automatically deleted when the counting is 0.

	shared_ptr<T> sptr = make_shared<T>(T_constructor_params ...);	// make_shared allocates the object on the heap

	Moreover, there is no way to manage a stack allocated object with a shared pointer!
	*/

	class CompositeHittable : public Hittable
	{

	public:

		// Constructors:

		CompositeHittable()
		{

		}

		CompositeHittable(std::shared_ptr<Hittable> hittable_object)
		{
			components.push_back(hittable_object);
		}

		// Methods:

		void clear()
		{
			components.clear();		// Note that this destroys the (shared) pointers
		}

		void add(std::shared_ptr<Hittable> hittable_object)
		{
			components.push_back(hittable_object);
		}

		virtual bool is_hit_by(const Ray& ray, float t_min, float t_max, HitRecord& record) const override
		{
			HitRecord component_record;
			bool is_hit = false;
			float visibility = t_max;

			for (const auto& component : components)
			{
				if (component->is_hit_by(ray, t_min, visibility, component_record))
				{
					is_hit = true;
					visibility = component_record.t;
					record = component_record;
				}
			}

			return is_hit;
		}

	private:

		std::vector<std::shared_ptr<Hittable>> components;

	};

	class Sphere : public Hittable
	{
	public:

		Sphere()
		{

		}

		Sphere(Point3D centre, float r, std::shared_ptr<Material> material)
			: center{ centre }, radius{ r }, material_pointer{ material }
		{

		}

		virtual bool is_hit_by(const Ray& ray, float t_min, float t_max, HitRecord& record) const override
		{
			glm::vec3 sphere_factor = ray.origin() - center;
			float A = glm::dot(ray.direction(), ray.direction());
			float half_B = glm::dot(ray.direction(), sphere_factor);
			float C = glm::dot(sphere_factor, sphere_factor) - radius * radius;
			float discriminant = half_B * half_B - A * C;

			if (discriminant < 0.0f)		// at this stage, discriminant == 0 case isn't rejected.
			{
				return false;
			}

			float dist = std::sqrt(discriminant);
			float root = ((-half_B) - dist) / A;	// the "near" root
			if (root < t_min || t_max < root)
			{
				root = ((-half_B) + dist) / A;		// the "far" root
				if (root < t_min || t_max < root)	// in-range far root is ACCEPTED when near root is rejected
				{
					return false;
				}
			}
			// record the intersection:
			record.t = root;
			record.point = ray.at(root);
			record.set_normal(ray, (record.point - center) / radius);	// We decide to let normal be unit vector!!!
			record.material_pointer = material_pointer;

			return true;
		}

	private:

		Point3D center{ 0.0f,0.0f,0.0f };
		float radius = 0.0f;
		std::shared_ptr<Material> material_pointer;
	};
}

#endif // !HITTABLE_H