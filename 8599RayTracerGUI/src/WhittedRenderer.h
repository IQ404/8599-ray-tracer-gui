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

namespace Whitted
{
	struct Payload
	{
		Entity* entity_hitted;
		uint32_t triangle_index;
		glm::vec2 barycentric_coordinates;
		float t;
	};

	class WhittedRayTracer
	{
	public:
		void Render(const World& world);
	};
}

#endif // !WHITTEDRENDERER_H
