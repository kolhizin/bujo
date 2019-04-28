#pragma once
#include <xtensor/xtensor.hpp>
#include <vector>

namespace bujo
{
	namespace curves
	{
		struct Curve
		{
			xt::xtensor<float, 1> len_param;
			xt::xtensor<float, 1> x_value, y_value;

			inline bool check() const
			{
				return (len_param.size() == x_value.size()) && (len_param.size() == y_value.size());
			}
		};

		struct CurveGenerationOptions
		{
			float max_angle = 1.0f; //in radians
		};

		std::vector<std::tuple<unsigned, unsigned>> selectSupportPoints(const xt::xtensor<float, 2>& src,
			unsigned num_points, float quantile_v, float quantile_h);

		Curve generateCurve(const xt::xtensor<float, 2>& src, CurveGenerationOptions options);
	}
}