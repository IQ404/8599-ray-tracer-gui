/*****************************************************************//**
 * \file   Scene.h
 * \brief  Representation of data in the world
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <glm/glm.hpp>

struct Material
{
	glm::vec3 albedo{ 1.0 };
	float roughness{ 1.0f };
	float metallic{ 0.0f };
};

struct Sphere
{
	glm::vec3 center{ 0.0f,0.0f,0.0f };
	float radius{ 0.5 };
	int material_index = 0;
};

struct Scene
{
	std::vector<Sphere> spheres;
	std::vector<Material> materials;
};

#endif // !SCENE_H