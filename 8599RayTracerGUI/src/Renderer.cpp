/*****************************************************************//**
 * \file   Renderer.cpp
 * \brief  The definitions of the renderer class for the 8599 ray tracer
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#include "Renderer.h"
//#include "WhittedRenderer.h"
#include "Sphere.h"
#include "TriangleMesh.h"

namespace RTUtility
{
	uint32_t vecRGBA_to_0xABGR(const glm::vec4& colorRGBA)
	{
		uint8_t r = (uint8_t)(colorRGBA.r * 255.0f);
		uint8_t g = (uint8_t)(colorRGBA.g * 255.0f);
		uint8_t b = (uint8_t)(colorRGBA.b * 255.0f);
		uint8_t a = (uint8_t)(colorRGBA.a * 255.0f);

		return ((a << 24) | (b << 16) | (g << 8) | r);	// this automatically promotes 8 bits memory to 32 bits memory
	}
}

Renderer::Renderer()
{
	auto diffuse_sphere = std::make_unique<Whitted::Sphere>(glm::vec3(-1, 0, -12), 2.0f);
	diffuse_sphere->material_nature = Whitted::Diffuse_Glossy;
	diffuse_sphere->diffuse_color = glm::vec3(0.6, 0.7, 0.8);
	world.Add(std::move(diffuse_sphere));

	auto glass_sphere = std::make_unique<Whitted::Sphere>(glm::vec3(0.5, -0.5, -8), 1.5f);
	glass_sphere->material_nature = Whitted::Reflective_Refractive;
	glass_sphere->refractive_index = 1.5;
	world.Add(std::move(glass_sphere));

	glm::vec3 vertices[4] = { {-5,-3,-6}, {5,-3,-6}, {5,-3,-16}, {-5,-3,-16} };
	uint32_t verticesIndices[6] = { 0, 1, 3, 1, 2, 3 };
	glm::vec2 texture_coordinates[4] = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };
	auto chessboard = std::make_unique<Whitted::TriangleMesh>(vertices, verticesIndices, 2, texture_coordinates);
	// Note: currently I don't want geometry that isn't closed (i.e. does not have an interior) to be reflective/refractive
	chessboard->material_nature = Whitted::Diffuse_Glossy;
	world.Add(std::move(chessboard));

	world.Add(std::make_unique<Whitted::PointLightSource>(glm::vec3(-20.0f, 70.0f, 20.0f), glm::vec3(0.5f)));
	world.Add(std::make_unique<Whitted::PointLightSource>(glm::vec3(30.0f, 50.0f, -12.0f), glm::vec3(0.5f)));
}

void Renderer::ResizeViewport(uint32_t width, uint32_t height)
{
	if (frame_image_final)
	{
		if ((frame_image_final->GetWidth() == width) && (frame_image_final->GetHeight() == height))
		{
			return;
		}
		frame_image_final->Resize(width, height);
	}
	else
	{
		frame_image_final = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	delete[] frame_data;
	delete[] temporal_accumulation_frame_data;
	frame_data = new uint32_t[width * height];
	temporal_accumulation_frame_data = new glm::vec4[width * height];
	frame_accumulating = 1;

	rows.resize(height);
	for (uint32_t i = 0; i < height; i++)
	{
		rows[i] = i;
	}
	columns.resize(width);
	for (uint32_t i = 0; i < width; i++)
	{
		columns[i] = i;
	}
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	active_scene = &scene;
	active_camera = &camera;

	if (frame_accumulating == 1)
	{
		std::memset(temporal_accumulation_frame_data, 0, frame_image_final->GetWidth() * frame_image_final->GetHeight() * sizeof(glm::vec4));
	}

	std::for_each(std::execution::par, rows.begin(), rows.end(),
		[this](uint32_t y)
		{
			std::for_each(std::execution::par, columns.begin(), columns.end(),
			[this, y](uint32_t x)
				{
					RayGen_Shader(x, y);
				}
			);
		}
	);

	frame_image_final->SetData(frame_data);	// send the frame data to GPU

	if (settings.accumulating)
	{
		frame_accumulating++;
	}
	else
	{
		frame_accumulating = 1;
	}
}

void Renderer::RayGen_Shader(uint32_t x, uint32_t y)
{
	glm::vec3 color_rgb(0.0f);

	/*Ray ray;
	ray.origin = active_camera->Position();
	ray.direction = active_camera->RayDirections()[y * frame_image_final->GetWidth() + x];*/

	//int bounces = 5;
	//float energy_remaining = 1.0f;
	//glm::vec3 sky_color{ 0.6f,0.7f,0.9f };
	//glm::vec3 light_direction = glm::normalize(glm::vec3{ -1.0,-1.0,-1.0 });
	//for (int i = 0; i < bounces; i++)
	//{
	//	Renderer::HitRecord hit_record = Intersection_Shader(ray);

	//	if (hit_record.hit_Distance == -1.0f)
	//	{
	//		color_rgb += sky_color * energy_remaining;
	//		break;
	//	}
	//	else
	//	{
	//		const Sphere& sphere = active_scene->spheres[hit_record.hit_ObjectIndex];
	//		const Material& material = active_scene->materials[sphere.material_index];
	//		float diffuseIntensity = glm::max(glm::dot(hit_record.hit_WorldNormal, -light_direction), 0.0f);
	//		color_rgb += energy_remaining * material.albedo * diffuseIntensity;	// light source emits white light

	//		energy_remaining *= 0.5f;
	//		
	//		ray.origin = hit_record.hit_WorldPosition + hit_record.hit_WorldNormal * 0.0001f;	// shadow acne elimination
	//		ray.direction = glm::reflect(ray.direction, hit_record.hit_WorldNormal + material.roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
	//	}
	//}

	glm::vec4 color_rgba{ Whitted::WhittedRayTracer::cast_Whitted_ray(active_camera->Position(), Whitted::normalize(active_camera->RayDirections()[y * frame_image_final->GetWidth() + x]), world, 0),1.0f };
	temporal_accumulation_frame_data[y * frame_image_final->GetWidth() + x] += color_rgba;
	glm::vec4 final_color_RGBA = temporal_accumulation_frame_data[y * frame_image_final->GetWidth() + x] / (float)frame_accumulating;
	final_color_RGBA = glm::clamp(final_color_RGBA, glm::vec4(0.0f), glm::vec4(1.0f));	// glm::clamp(value, min, max)

	frame_data[(y * frame_image_final->GetWidth()) + x] = RTUtility::vecRGBA_to_0xABGR(final_color_RGBA);
}

Renderer::HitRecord Renderer::Intersection_Shader(const Ray& ray)
{
	float closestT = std::numeric_limits<float>::max();
	int closest_sphere_index = -1;
	for (size_t index = 0; index < active_scene->spheres.size(); index++)
	{
		const Sphere& sphere = active_scene->spheres[index];
		glm::vec3 transformed_origin = ray.origin - sphere.center;	// Do the equivalent movement on camera rather than on sphere

		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.0f * glm::dot(transformed_origin, ray.direction);
		float c = glm::dot(transformed_origin, transformed_origin) - sphere.radius * sphere.radius;
		float discriminant = b * b - 4 * a * c;

		if (discriminant < 0.0f)
		{
			continue;
		}

		float closerT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
		if (closerT > 0.0f && closerT < closestT)	// note that here we are coding it so that when we are inside a sphere, that sphere will NOT be rendered.
		{
			closestT = closerT;
			closest_sphere_index = (int)index;
		}
	}
	if (closest_sphere_index == -1)
	{
		return Miss_Shader(ray);
	}
	return ClosestHit_Shader(ray, closestT, closest_sphere_index);
}

Renderer::HitRecord Renderer::ClosestHit_Shader(const Ray& ray, float hit_Distance, int hit_ObjectIndex)
{
	Renderer::HitRecord hit_record;
	hit_record.hit_Distance = hit_Distance;
	hit_record.hit_ObjectIndex = hit_ObjectIndex;
	
	const Sphere& closest_sphere = active_scene->spheres[hit_ObjectIndex];
	
	glm::vec3 transformed_origin = ray.origin - closest_sphere.center;	// Do the equivalent movement on camera rather than on sphere
	glm::vec3 transformed_hit_point = transformed_origin + ray.direction * hit_Distance;

	hit_record.hit_WorldNormal = glm::normalize(transformed_hit_point);
	hit_record.hit_WorldPosition = transformed_hit_point + closest_sphere.center;

	return hit_record;
}

Renderer::HitRecord Renderer::Miss_Shader(const Ray& ray)
{
	Renderer::HitRecord hit_record;
	hit_record.hit_Distance = -1.0f;
	return hit_record;
}