/*****************************************************************//**
 * \file   Material.h
 * \brief  The representations of materials used in the 8599 non-physical path tracer
 * 
 * \author Xiaoyang Liu
 * \date   July 2023
 *********************************************************************/

#ifndef MATERIAL_H
#define MATERIAL_H

#include "HitRecord.h"

namespace NP_PathTracing
{
	class Material
	{
	public:

		// The hacked BRDF:
		virtual bool scatter(const Ray& incident_ray, const HitRecord& record, glm::vec3& attenuation, Ray& scattered_ray) const = 0;
	};

	class Diffuse : public Material
	{
	public:

		Diffuse(const glm::vec3& _albedo)
			: albedo{ _albedo }
		{

		}

		virtual bool scatter(const Ray& incident_ray, const HitRecord& record, glm::vec3& attenuation, Ray& scattered_ray) const override
		{
			DiffuseModel diffuse_model = GetActiveDiffuseModel();
			glm::vec3 scattering_direction;

			switch (diffuse_model)
			{
			case NP_PathTracing::IN_Sphere:
				scattering_direction = record.normal + random_in_unit_sphere();
				break;
			case NP_PathTracing::ON_Sphere:
				scattering_direction = record.normal + random_unit_vector();	// ??? Explain why this (Lambertian) model is the most accurate one.
				break;
			case NP_PathTracing::IN_Hemisphere:
				scattering_direction = random_in_unit_hemisphere(record.normal);	// Note: this is equivalent to ON-hemisphere
				break;
			default:
				// TODO: initialize scattering_direction here
				break;
			}
			
			if (near_zero(scattering_direction))
			{
				scattering_direction = record.normal;
			}
			scattered_ray = Ray{ record.point, scattering_direction };
			attenuation = albedo;

			return true;
		}

	private:

		glm::vec3 albedo;	// for ColorRGB
	};

	class Metal : public Material
	{
	public:

		Metal(const glm::vec3& _albedo, float _fuzziness)
			: albedo{ _albedo }, fuzziness{ _fuzziness < 1.0f ? _fuzziness : 1.0f }
		{

		}

		virtual bool scatter(const Ray& incident_ray, const HitRecord& record, glm::vec3& attenuation, Ray& scattered_ray) const override
		{
			glm::vec3 reflection_direction = direction_of_mirror_reflection(glm::normalize(incident_ray.direction()), record.normal);
			scattered_ray = Ray{ record.point, reflection_direction + fuzziness * random_in_unit_sphere() };
			attenuation = albedo;
			return (glm::dot(reflection_direction, record.normal) > 0);	// absorb those energy with directions into the surface due to fuzziness (??? Is this physically accurate?)
		}

	private:

		glm::vec3 albedo;	// for ColorRGB
		float fuzziness;
	};

	class Dielectric : public Material
	{
	public:

		Dielectric(float refractive_index)
			: m_refractive_index{ refractive_index }
		{

		}

		virtual bool scatter(const Ray& incident_ray, const HitRecord& record, glm::vec3& attenuation, Ray& scattered_ray) const override
		{
			// currently the material refracts all the incident rays whenever it can.

			attenuation = glm::vec3{ 1.0,1.0,1.0 };	// assume no energy loss during refraction.reflection

			// Assume the given front face is pointing from the dielectric to AIR:
			float refraction_ratio = record.is_hitting_front_face ? (1.0f / m_refractive_index) : m_refractive_index;

			/*
			Note:
				If we (deliberately) pass a negative value into the radius, we will have is_hitting_front_face == false when the ray is hitting toward the negative-radius sphere,
				and we will have is_hitting_front_face == true when the ray is escaping the negative-radius sphere.
				Now, if this negative-radius sphere is put inside a normal sphere with positive radius, this negative-radius sphere will be acted as an air bubble (hollow) inside
				the normal sphere.
			*/

			glm::vec3 unit_incident_direction = glm::normalize(incident_ray.direction());
			float cos_theta = std::fmin(glm::dot(-unit_incident_direction, record.normal), 1.0);
			bool total_internal_reflection = ((refraction_ratio * std::sqrt(1.0 - cos_theta * cos_theta)) > 1.0);
			// Note: here std::fmin is to prevent error in floating point computation to pass a negative value into std::sqrt.

			if (total_internal_reflection || (specular_reflection_coefficient(cos_theta) > Walnut::Random::Float()))
			{
				scattered_ray = Ray{ record.point, direction_of_mirror_reflection(unit_incident_direction, record.normal) };
				return true;
			}
			scattered_ray = Ray{ record.point, direction_of_Snell_refraction(unit_incident_direction, record.normal, refraction_ratio) };
			return true;
		}

	private:

		float specular_reflection_coefficient(float cos_theta) const
		{
			// Schlick's approximation (reference: https://en.wikipedia.org/wiki/Schlick%27s_approximation)

			float r0 = ((1.0f - m_refractive_index) / (1.0f + m_refractive_index)) * ((1.0f - m_refractive_index) / (1.0f + m_refractive_index));
			return r0 + (1.0f - r0) * std::pow(1.0f - cos_theta, 5.0f);
		}

		float m_refractive_index;
	};

}

#endif // !MATERIAL_H