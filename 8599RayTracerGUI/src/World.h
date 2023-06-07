/*****************************************************************//**
 * \file   World.h
 * \brief  The representation of a Whitted-style ray-traced world
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef WORLD_H
#define WORLD_H


#include "VectorFloat.h"
#include "Entity.h"
#include "LightSource.h"
#include <memory>
#include <vector>

namespace Whitted
{
	class World
	{
	public:
		World(int width, int height)
			: m_width{ width }, m_height{ height }
		{

		}

		[[nodiscard]] const std::vector<std::unique_ptr<Entity>>& GetEntities() const
		{
			return entities;
		}
		// See https://oopscenities.net/2022/01/16/cpp17-nodiscard-attribute/#:~:text=C%2B%2B17%20adds%20a,or%20assigned%20to%20a%20value.
		[[nodiscard]] const std::vector<std::unique_ptr<PointLightSource>>& GetLightSources() const
		{
			return light_sources;
		}

		void Add(std::unique_ptr<Entity> entity)
		{
			entities.push_back(std::move(entity));
		}
		/*
		Note:
			the move constructor of std::unique_ptr constructs a unique_ptr by transferring the ownership (and nulls the previous owner).
		*/
		void Add(std::unique_ptr<PointLightSource> light_source)
		{
			light_sources.push_back(std::move(light_source));
		}

	public:
		int m_width = 1280;
		int m_height = 720;
		double fov = 90;	// TODO: vertical?
		glm::vec3 sky_color{0.235294f, 0.67451f, 0.843137f};
		int max_bounce_depth = 5;
		float intersection_correction = 0.00001f;
	private:
		std::vector<std::unique_ptr<Entity>> entities;
		std::vector<std::unique_ptr<PointLightSource>> light_sources;
	};
}

#endif // !WORLD_H