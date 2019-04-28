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
			float rel_step_size = 0.05f; //in percent of image width
			float rel_window_size_x = 0.05f; //in percent of image width
			float rel_window_size_y = 0.05f; //in percent of image height
			unsigned num_radon_angles = 51; //odd to include zero
			unsigned num_radon_offsets = 50;
			float angle_regularization_coef = 0.1f;
			float angle_regularization_power = 0.25f;
		};
		
		std::vector<std::tuple<unsigned, unsigned>> selectSupportPoints(const xt::xtensor<float, 2>& src,
			unsigned num_points, float quantile_v, float quantile_h);

		Curve generateCurve(const xt::xtensor<float, 2>& src, int i0, int j0, const CurveGenerationOptions& options);
	}
}