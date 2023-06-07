/*****************************************************************//**
 * \file   Renderer.h
 * \brief  The header file of the renderer for the 8599 ray tracer
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <execution>
#include <memory>	// to use std::shared_ptr
#include <glm/glm.hpp>		// to use glm vec
#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include "WhittedRenderer.h"

class Renderer
{
public:		// structs

	struct Settings
	{
		bool accumulating = true;
	};

public:		// methods

	//Renderer() = default;	// Defaulted default constructor: the compiler will define the implicit default constructor even if other constructors are present.
	Renderer();

	void ResizeViewport(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const
	{
		return frame_image_final;
	}

	void Reaccumulate()
	{
		frame_accumulating = 1;
	}

	Settings& GetSettings()
	{
		return settings;
	}

private:	// structs

	struct HitRecord
	{
		float hit_Distance;
		glm::vec3 hit_WorldPosition;
		glm::vec3 hit_WorldNormal;

		int hit_ObjectIndex;
	};

private:	// methods

	void RayGen_Shader(uint32_t x, uint32_t y);	// mimic one of the vulkan shaders which is called to cast ray(s) for every pixel
	HitRecord Intersection_Shader(const Ray& ray);		// Intersection_Shader: mimic one of the vulkan shaders which is called to determine whether we hit something
	HitRecord ClosestHit_Shader(const Ray& ray, float hit_Distance, int hit_ObjectIndex);	// mimic one of the vulkan shaders which is called when the ray hits something
	HitRecord Miss_Shader(const Ray& ray);	// mimic one of the vulkan shaders which is called when the ray does not hit anything

private:	// members
	Settings settings;
	std::vector<uint32_t> rows;
	std::vector<uint32_t> columns;
	std::shared_ptr<Walnut::Image> frame_image_final;
	uint32_t* frame_data = nullptr;
	glm::vec4* temporal_accumulation_frame_data = nullptr;
	uint32_t frame_accumulating = 1;	// the index (starting from 1) of the current frame that is being accumulated into the temporal_accumulation_frame_data buffer
	const Scene* active_scene = nullptr;
	const Camera* active_camera = nullptr;

	Whitted::World world{1280, 960};
};

#endif // !RENDERER_H