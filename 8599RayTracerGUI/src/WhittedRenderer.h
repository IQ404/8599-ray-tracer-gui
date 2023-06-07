/*****************************************************************//**
 * \file   WhittedRenderer.h
 * \brief  The specific renderer for rendering Whitted-style ray-traced scenes
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef WHITTEDRENDERER_H
#define WHITTEDRENDERER_H

#include "World.h"
#include <optional>

namespace Whitted
{
	struct Payload
	{
		Entity* entity_hitted;
		uint32_t triangle_index;
		glm::vec2 barycentric_coordinates;
		float t;
	};

	// Average index of refractions:
#define eta_Vacuum 1.0
#define eta_Air 1.00029
#define eta_20C_Water 1.333
#define eta_Glass1 1.5
#define eta_Glass2 1.6
#define eta_Diamond 2.42

	inline glm::vec3 mirror_reflection_direction(const glm::vec3& incident_ray_direction, const glm::vec3& surface_normal)
	{
		return incident_ray_direction - 2 * glm::dot(incident_ray_direction, surface_normal) * surface_normal;
		// Note: this algorithm works even when incident ray is coming from inside (surface normal is always pointing outwards).
	}

	inline glm::vec3 snell_refraction_direction(const glm::vec3& incident_ray_direction, const glm::vec3& surface_normal, const float& entity_refraction_index)
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

	inline float accurate_fresnel_reflectance(const glm::vec3& incident_ray_direction, const glm::vec3& surface_normal, const float& entity_refraction_index)
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

	inline std::optional<Whitted::Payload> get_intersection_payload(
		const glm::vec3& ray_origin,
		const glm::vec3& ray_direction,
		const std::vector<std::unique_ptr<Entity>>& entities
	)
		// Empty std::optional<Whitted::Payload> means no intersection
	{
		std::optional<Whitted::Payload> payload{};
		float t_closest = Whitted::positive_infinity;
		for (const std::unique_ptr<Entity>& entity : entities)
		{
			float t_local = Whitted::positive_infinity;
			uint32_t local_triangle_index;
			glm::vec2 local_barycentric_coordinates;
			if (
				entity->Intersect(ray_origin, ray_direction, t_local, local_triangle_index, local_barycentric_coordinates)
				&&
				t_local < t_closest
				)
			{
				payload.emplace();
				t_closest = t_local;
				payload->t = t_closest;
				payload->entity_hitted = entity.get();
				payload->triangle_index = local_triangle_index;
				payload->barycentric_coordinates = local_barycentric_coordinates;
			}
		}
		return payload;
	}
	// For std::optional, see 15.4.2, Bjarne Stroustrup's A Tour of C++ (3rd ed.).
	// It is essentially a pointer that can manage an stack object that is default-initialized (i.e. {}) to nullptr.
	//
	// std::optional<T>::emplace - Constructs the contained value in-place. If *this already contains a value before
	// the call, the contained value is destroyed by calling its destructor.

	class WhittedRayTracer
	{
	public:
		static glm::vec3 cast_Whitted_ray(
			const glm::vec3& ray_origin,
			const glm::vec3& ray_direction,
			const Whitted::World& world,
			int has_already_bounced
		)
		{
			if ((has_already_bounced > world.max_bounce_depth) || (ray_direction == glm::vec3{0.0f, 0.0f, 0.0f}))	// TODO: should we check ray_direction here?
			{
				return glm::vec3{0.0f, 0.0f, 0.0f};		// no energy received
			}
			glm::vec3 ray_color = world.sky_color;
			// here we are computing the color carried by (this particular) one ray,
			// NOT one path (which, I assume, is a collection of rays linked by the hitting points).
			if (std::optional<Whitted::Payload> payload = get_intersection_payload(ray_origin, ray_direction, world.GetEntities()))
			{
				glm::vec3 intersection = ray_origin + ray_direction * payload->t;
				glm::vec3 normal_at_intersection;
				glm::vec2 texture_coordinates_at_intersection;
				payload->entity_hitted->GetHitInfo(
					intersection,
					ray_direction,
					payload->triangle_index,
					payload->barycentric_coordinates,
					normal_at_intersection,
					texture_coordinates_at_intersection
				);

				switch (payload->entity_hitted->material_nature)
				{
				case Whitted::Reflective:
				{
					glm::vec3 reflected_ray_direction = Whitted::normalize(Whitted::mirror_reflection_direction(ray_direction, normal_at_intersection));
					glm::vec3 reflected_ray_origin = (glm::dot(reflected_ray_direction, normal_at_intersection) < 0.0f) ?
						(intersection - normal_at_intersection * world.intersection_correction)
						:
						(intersection + normal_at_intersection * world.intersection_correction);
					// TODO: conductors have complex refractive index
					ray_color = cast_Whitted_ray(reflected_ray_origin, reflected_ray_direction, world, has_already_bounced + 1) *
						Whitted::accurate_fresnel_reflectance(-reflected_ray_direction, normal_at_intersection, payload->entity_hitted->refractive_index);
					break;
				}
				case Whitted::Reflective_Refractive:
				{
					glm::vec3 reflected_ray_direction = Whitted::normalize(
						Whitted::mirror_reflection_direction(ray_direction, normal_at_intersection)
					);
					glm::vec3 reflected_ray_origin = (glm::dot(reflected_ray_direction, normal_at_intersection) < 0.0f) ?
						(intersection - normal_at_intersection * world.intersection_correction)
						:
						(intersection + normal_at_intersection * world.intersection_correction);

					glm::vec3 refracted_ray_direction = Whitted::normalize(
						Whitted::snell_refraction_direction(ray_direction, normal_at_intersection, payload->entity_hitted->refractive_index)
					);
					glm::vec3 refracted_ray_origin = (glm::dot(refracted_ray_direction, normal_at_intersection) < 0.0f) ?
						(intersection - normal_at_intersection * world.intersection_correction)
						:
						(intersection + normal_at_intersection * world.intersection_correction);

					glm::vec3 reflected_ray_color = cast_Whitted_ray(reflected_ray_origin, reflected_ray_direction, world, has_already_bounced + 1);
					glm::vec3 refracted_ray_color = cast_Whitted_ray(refracted_ray_origin, refracted_ray_direction, world, has_already_bounced + 1);

					float reflectance = Whitted::accurate_fresnel_reflectance(ray_direction, normal_at_intersection, payload->entity_hitted->refractive_index);
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

					glm::vec3 shading_point = (glm::dot(ray_direction, normal_at_intersection) < 0.0f) ?
						(intersection + normal_at_intersection * world.intersection_correction)
						:
						(intersection - normal_at_intersection * world.intersection_correction);

					for (const auto& light_source : world.GetLightSources())
					{
						glm::vec3 light_source_direction = light_source->m_position - intersection;
						float light_distance_squared = glm::dot(light_source_direction, light_source_direction);
						light_source_direction = Whitted::normalize(light_source_direction);

						std::optional<Whitted::Payload> potential_occlusion = Whitted::get_intersection_payload(shading_point, light_source_direction, world.GetEntities());
						// Note here that if we are inside the shaded object and the light source is outside the shaded object,
						// then the shaded point is always occluded by the shaded object itself.
						if (potential_occlusion && (potential_occlusion->t * potential_occlusion->t < light_distance_squared))	// is occluded
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
										Whitted::mirror_reflection_direction(-light_source_direction, normal_at_intersection),
										ray_direction)
								), payload->entity_hitted->specular_size_factor
							) * light_source->m_radiance;
						}
					}

					ray_color = total_radiance_diffuse * payload->entity_hitted->GetDiffuseColor(texture_coordinates_at_intersection) * payload->entity_hitted->phong_diffuse
						+
						total_radiance_specular /* assume specular color == (1,1,1) */ * payload->entity_hitted->phong_specular;

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
	};
}

#endif // !WHITTEDRENDERER_H
