/*****************************************************************//**
 * \file   WhittedMaterial.h
 * \brief  The class to store material information for objects to be Whitted-style ray traced
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef WHITTED_MATERIAL_H
#define WHITTED_MATERIAL_H

#include "VectorFloat.h"

namespace Whitted
{
	enum MaterialNature
	{
		Reflective,
		Reflective_Refractive,
		Diffuse_Glossy
	};

	class WhittedMaterial
	{
	public:
		WhittedMaterial(
			MaterialNature material_nature = Diffuse_Glossy,
			glm::vec3 diffuse_color = glm::vec3{ 1.0f,1.0f,1.0f },
			glm::vec3 emission = glm::vec3{ 0.0f,0.0f,0.0f }
		)
		{
			m_material_nature = material_nature;
			m_diffuse_color = diffuse_color;
			m_emission = emission;
		}

		MaterialNature GetMaterialNature()
		{
			return m_material_nature;
		}

		glm::vec3 GetDiffuseColor()
		{
			return m_diffuse_color;
		}

	public:
		MaterialNature m_material_nature;
		float refractive_index = 1.0f;
		float phong_diffuse;
		float phong_specular;
		glm::vec3 m_diffuse_color;
		float specular_size_factor;		// The larger this factor is, the SMALLER the specular size will be.
										// Because this will be used as an exponent.
		glm::vec3 m_emission;
	};
}

#endif // !WHITTED_MATERIAL_H
