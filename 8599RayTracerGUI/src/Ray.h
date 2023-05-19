/*****************************************************************//**
 * \file   Ray.h
 * \brief  Data structure representing the ray
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

#endif // !RAY_H