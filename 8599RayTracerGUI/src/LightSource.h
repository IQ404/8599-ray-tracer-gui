/*****************************************************************//**
 * \file   Light.h
 * \brief  The representations of light sources in Whitted Style ray tracing
 * 
 * \author Xiaoyang Liu
 * \date   June 2023
 *********************************************************************/

#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include "WhittedUtilities.h"
#include "VectorFloat.h"

namespace Whitted
{
	class PointLightSource
	{
	public:
		PointLightSource(const glm::vec3& light_source_origin, const glm::vec3& radiance)
			: m_light_source_origin{ light_source_origin }, m_radiance{ radiance }
		{

		}

		virtual ~PointLightSource() = default;

	public:
		glm::vec3 m_light_source_origin;
		glm::vec3 m_radiance;
	};

	class AreaLightSource : public PointLightSource
	{
	public:
		AreaLightSource(const glm::vec3& light_source_origin, const glm::vec3& radiance)
			: PointLightSource(light_source_origin, radiance)
		{
			// TODO
			m_u = glm::vec3{ 1, 0, 0 };
			m_v = glm::vec3{ 0, 0, 1 };
		}

		glm::vec3 GetRandomSample() const
		{
			return m_light_source_origin + get_random_float_0_1() * m_u + get_random_float_0_1() * m_v;
		}

	public:		// Data Members:
		// TODO
		glm::vec3 m_u;
		glm::vec3 m_v;
	};
}

#endif // !LIGHTSOURCE_H