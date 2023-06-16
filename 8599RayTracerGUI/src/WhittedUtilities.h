/*****************************************************************//**
 * \file   WhittedUtilities.h
 * \brief  Utility functions and constants for Whitted style ray tracing
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef WHITTEDUTILITIES_H
#define WHITTEDUTILITIES_H

#include <utility>
#include "Walnut/Random.h"
#include <cmath>

namespace Whitted
{
	#define INTERSECTION_CORRECTION 0.00001f
	#undef PI
	#define PI 3.141592653589793f
	constexpr float positive_infinity = std::numeric_limits<float>::max();

	inline float get_random_float_0_1()
	{
		return Walnut::Random::Float();
	}

	inline float clamp_float(const float& value, const float& lower_bound, const float& upper_bound)
	{
		return std::max(std::min(value, upper_bound), lower_bound);
	}

	inline bool QuadraticFormula(const float& A, const float& B, const float& C, float& x_small, float& x_large)
	{
		float discriminant = B * B - 4 * A * C;
		if (discriminant < 0)
		{
			return false;
		}
		else if (discriminant == 0)
		{
			x_small = x_large = -0.5 * B / A;
		}
		else
		{	// See https://mathworld.wolfram.com/QuadraticEquation.html
			float q = (B > 0) ? (-0.5 * (B + std::sqrt(discriminant))) : (-0.5 * (B - std::sqrt(discriminant)));
			x_small = q / A;	// A != 0 for quadratic equation
			x_large = C / q;
		}
		if (x_small > x_large)
		{
			std::swap(x_small, x_large);
		}
		return true;
	}

	inline float Degree_to_Radian(const float& degree)
	{
		return degree * PI / 180.0;
	}
}


#endif // !WHITTEDUTILITIES_H
