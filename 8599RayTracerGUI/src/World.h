/*****************************************************************//**
 * \file   World.h
 * \brief  The representation of a Whitted-style ray-traced world
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef WORLD_H
#define WORLD_H

#include <memory>
#include <vector>

#include "BVH.h"
#include "LightSource.h"


namespace Whitted
{
	class World
	{
		// Average index of refractions:
		#define eta_Vacuum 1.0
		#define eta_Air 1.00029
		#define eta_20C_Water 1.333
		#define eta_Glass1 1.5
		#define eta_Glass2 1.6
		#define eta_Diamond 2.42

	public:
		World(int width, int height)
			: m_width{ width }, m_height{ height }
		{

		}

		~World()
		{
			delete bvh;
		}

		[[nodiscard]] const std::vector<Entity*>& GetEntities() const
		{
			return entities;
		}
		// See https://oopscenities.net/2022/01/16/cpp17-nodiscard-attribute/#:~:text=C%2B%2B17%20adds%20a,or%20assigned%20to%20a%20value.
		[[nodiscard]] const std::vector<std::unique_ptr<PointLightSource>>& GetLightSources() const
		{
			return light_sources;
		}

		void Add(Entity* entity_pointer)
		{
			entities.push_back(entity_pointer);
		}
		
		void Add(std::unique_ptr<PointLightSource> light_source)
		{
			light_sources.push_back(std::move(light_source));
			/*
			Note:
				the move constructor of std::unique_ptr constructs a unique_ptr by transferring the ownership
				(and nulls the previous owner).
			*/
		}

		void GenerateBVH()
		{
			bvh = new AccelerationStructure::BVH{ entities };	// put into constructor? NO, since we may add entities before rendering but after initializing the World
		}

		IntersectionRecord ray_BVH_intersection_record(const AccelerationStructure::Ray& ray) const
		{
			return bvh->traverse_BVH_from_root(ray);
		}

		glm::vec3 mirror_reflection_direction(const glm::vec3& incident_ray_direction, const glm::vec3& surface_normal) const
		{
			return incident_ray_direction - 2 * glm::dot(incident_ray_direction, surface_normal) * surface_normal;
			// Note: this algorithm works even when incident ray is coming from inside (surface normal is always pointing outwards).
		}

		glm::vec3 snell_refraction_direction(const glm::vec3& incident_ray_direction, const glm::vec3& surface_normal, const float& entity_refraction_index) const
			// Assume incident_ray_direction and surface_normal are unit vectors where the incident_ray_direction is toward to the
			// surface evaluation point and the surface_nornal is pointing outwards.
			// Assume we want the normal used in computation to always towards the incident ray, and want the incident vector used in
			// computation to always originate from the surface evaluation point.
			// The snell_refraction_direction we are returning is also a unit vector.
		{
			// Trial settings:
			float eta_in = eta_Vacuum;
			float eta_out = entity_refraction_index;
			glm::vec3 normal = surface_normal;
			// Trial settings correction:
			float cos_incident = Whitted::clamp_float(glm::dot(incident_ray_direction, surface_normal), -1, 1);
			if (cos_incident < 0)		// case 1: ray is coming from the outside
			{
				cos_incident = -cos_incident;
			}
			else	// case 2: ray is coming from the inside
					// Note that if cos_incident = 0, we treat the incident ray as coming from the inside, thus we return 0
			{
				std::swap(eta_in, eta_out);
				normal = -normal;
			}
			float eta_ratio = eta_in / eta_out;
			float cos_refract_squared = 1 - eta_ratio * eta_ratio * (1 - cos_incident * cos_incident);
			return (cos_refract_squared < 0) ?
				(glm::vec3{0.0f, 0.0f, 0.0f}) :
				(eta_ratio * incident_ray_direction + (eta_ratio * cos_incident - std::sqrtf(cos_refract_squared)) * normal);
			// if not total internal reflection, we have:
			//		refraction_direction = component_parallel_to_the_normal + component_perpendicular_to_the_normal
		}

		float accurate_fresnel_reflectance(const glm::vec3& incident_ray_direction, const glm::vec3& surface_normal, const float& entity_refraction_index) const
			// The parameters has the same assumptions as for snell_refraction_direction().
		{
			// Trial settings:
			float eta_in = eta_Vacuum;
			float eta_out = entity_refraction_index;
			// Trial settings correction:
			float cos_incident = Whitted::clamp_float(glm::dot(incident_ray_direction, surface_normal), -1, 1);
			if (cos_incident < 0)		// case 1: ray is coming from the outside
			{
				cos_incident = -cos_incident;
			}
			else	// case 2: ray is coming from the inside
					// Note that if cos_incident = 0 we treat the incident ray as coming from the inside, thus we return 1
			{
				std::swap(eta_in, eta_out);
			}
			float sin_refract = eta_in / eta_out * std::sqrtf(std::max(0.0f, 1 - cos_incident * cos_incident));		// Snell's law
			if (sin_refract > 1.0f)		// Total internal reflection
			{
				return 1.0f;
			}
			else	// Note: this is NOT Schlick¡¯s approximation, we DO consider polarization here
			{
				float cos_refract = std::sqrtf(std::max(0.0f, 1 - sin_refract * sin_refract));
				float R_s_sqrt = (eta_in * cos_incident - eta_out * cos_refract) / (eta_in * cos_incident + eta_out * cos_refract);
				float R_p_sqrt = (eta_in * cos_refract - eta_out * cos_incident) / (eta_in * cos_refract + eta_out * cos_incident);
				return (R_s_sqrt * R_s_sqrt + R_p_sqrt * R_p_sqrt) / 2;
				// See https://en.wikipedia.org/wiki/Fresnel_equations
			}
		}

		glm::vec3 cast_Whitted_ray(const AccelerationStructure::Ray& ray, int has_already_bounced) const
		{
			if ((has_already_bounced > max_bounce_depth) || (ray.m_direction == glm::vec3{0.0f, 0.0f, 0.0f}))	// TODO: should we check ray_direction here?
			{
				return glm::vec3{0.0f, 0.0f, 0.0f};		// no energy received
			}
			glm::vec3 ray_color = sky_color;
			IntersectionRecord record = ray_BVH_intersection_record(ray);
			if (record.has_intersection)
			{
				glm::vec3 intersection = record.location;
				glm::vec3 normal_at_intersection = record.surface_normal;
				switch (record.hitted_entity_material->GetMaterialNature())
				{
				case Whitted::Reflective:
				{
					glm::vec3 reflected_ray_direction = Whitted::normalize(mirror_reflection_direction(ray.m_direction, normal_at_intersection));
					glm::vec3 reflected_ray_origin = (glm::dot(reflected_ray_direction, normal_at_intersection) < 0.0f) ?
						(intersection - normal_at_intersection * INTERSECTION_CORRECTION)
						:
						(intersection + normal_at_intersection * INTERSECTION_CORRECTION);
					// TODO: conductors have complex refractive index
					ray_color = cast_Whitted_ray(AccelerationStructure::Ray{reflected_ray_origin, reflected_ray_direction}, has_already_bounced + 1) *
								accurate_fresnel_reflectance(-reflected_ray_direction, normal_at_intersection, record.hitted_entity_material->refractive_index);
					break;
				}
				case Whitted::Reflective_Refractive:
				{
					glm::vec3 reflected_ray_direction = Whitted::normalize(
						mirror_reflection_direction(ray.m_direction, normal_at_intersection)
					);
					glm::vec3 reflected_ray_origin = (glm::dot(reflected_ray_direction, normal_at_intersection) < 0.0f) ?
						(intersection - normal_at_intersection * INTERSECTION_CORRECTION)
						:
						(intersection + normal_at_intersection * INTERSECTION_CORRECTION);

					glm::vec3 refracted_ray_direction = Whitted::normalize(
						snell_refraction_direction(ray.m_direction, normal_at_intersection, record.hitted_entity_material->refractive_index)
					);
					glm::vec3 refracted_ray_origin = (glm::dot(refracted_ray_direction, normal_at_intersection) < 0.0f) ?
						(intersection - normal_at_intersection * INTERSECTION_CORRECTION)
						:
						(intersection + normal_at_intersection * INTERSECTION_CORRECTION);

					glm::vec3 reflected_ray_color = cast_Whitted_ray(AccelerationStructure::Ray{reflected_ray_origin, reflected_ray_direction}, has_already_bounced + 1);
					glm::vec3 refracted_ray_color = cast_Whitted_ray(AccelerationStructure::Ray{refracted_ray_origin, refracted_ray_direction}, has_already_bounced + 1);

					float reflectance = accurate_fresnel_reflectance(ray.m_direction, normal_at_intersection, record.hitted_entity_material->refractive_index);
					// !!! Note that here we use reciprocity of light (in which I will be really happy if we can find an asymmetry to break it and thus publish a Siggraph :) ):
					ray_color = reflectance * reflected_ray_color + (1.0f - reflectance) * refracted_ray_color;
					break;
				}
				default:	// Diffuse_Glossy
					/*
					Notes:
					The path ends when hitting on any entity in the world whose material is defined to be Diffuse_Glossy and we assume all
					the lights coming from that end point can be shaded by a simplified Blinn-Phong model (by simple I mean to discard
					ambient term and the effect of energy dispersing with distance).
					*/
				{
					glm::vec3 total_radiance_diffuse{0.0f, 0.0f, 0.0f};
					glm::vec3 total_radiance_specular{0.0f, 0.0f, 0.0f};

					glm::vec3 shading_point = (glm::dot(ray.m_direction, normal_at_intersection) < 0.0f) ?
						(intersection + normal_at_intersection * INTERSECTION_CORRECTION)
						:
						(intersection - normal_at_intersection * INTERSECTION_CORRECTION);

					for (const auto& light_source : light_sources)
					{
						AreaLightSource* area_light_source = dynamic_cast<AreaLightSource*>(light_source.get());
						/*
						C++ side-note:
						If the cast is successful, dynamic_cast returns a value of type target-type.
						If the cast fails and target-type is a pointer type, it returns a null pointer of that type.
						*/
						if (area_light_source)
						{
							// TODO
						}
						else    // point light
						{
							glm::vec3 light_source_direction = light_source->m_light_source_origin - intersection;
							float light_distance_squared = glm::dot(light_source_direction, light_source_direction);
							light_source_direction = Whitted::normalize(light_source_direction);

							IntersectionRecord shadow_record = bvh->traverse_BVH_from_root(AccelerationStructure::Ray{shading_point, light_source_direction});
							// Note here that if we are inside the shaded object and the light source is outside the shaded object,
							// then the shaded point is always occluded by the shaded object itself.
							if ((shadow_record.has_intersection) && (shadow_record.t * shadow_record.t < light_distance_squared))	// is occluded
							{
								continue;
								// should we still have specular illumination here?
							}
							else
							{
								total_radiance_diffuse += light_source->m_radiance * std::fabsf(glm::dot(light_source_direction, normal_at_intersection));
								total_radiance_specular += std::powf(
									std::max(0.0f,
										-glm::dot(
											mirror_reflection_direction(-light_source_direction, normal_at_intersection),
											ray.m_direction)
									), record.hitted_entity_material->refractive_index
								) * light_source->m_radiance;
								// specularExponent = 0 means, every shading point where the cosine is not less/equal to 0 will have the same
								// specular color.
							}
						}
					}

					ray_color = total_radiance_diffuse * record.hitted_entity->GetDiffuseColor() * record.hitted_entity_material->phong_diffuse
						+
						total_radiance_specular /* assume specular color == (1,1,1) */ * record.hitted_entity_material->phong_specular;

					break;
					/*
					Some questions:
					1. Once in shadow, should we still have specular illumination?
					2. What if the occluding entity is transparent (e.g. glass)?
					3. No need to consider cosine law for specular illumination?
					*/
				} // The end of the default statement
				} // The end of the switch statement
			}
			return ray_color;
		}

	public:
		int m_width = 1280;
		int m_height = 720;
		double fov = 90;	// TODO: vertical? Not needed?
		glm::vec3 sky_color{0.235294f, 0.67451f, 0.843137f};
		int max_bounce_depth = 5;
		//float intersection_correction = 0.00001f;
		AccelerationStructure::BVH* bvh = nullptr;
		std::vector<Entity*> entities;
		std::vector<std::unique_ptr<PointLightSource>> light_sources;
	};
}

#endif // !WORLD_H