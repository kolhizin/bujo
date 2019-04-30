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
			void calculateLenParametrization();
		};

		namespace interpolate
		{
			std::tuple<xt::xtensor<float, 1>, xt::xtensor<float, 1>> getDenseXY(const Curve& curve);
		}

		namespace integral
		{
			float calcIntegralOverCurve(const xt::xtensor<float, 2>& src, const Curve& curve, float offset = 0.0f);
			xt::xtensor<float, 1> calcIntegralOverCurve(const xt::xtensor<float, 2>& src, const Curve& curve, const xt::xtensor<float, 1> &offsets);
			xt::xtensor<float, 1> calcAccumIntegralOverCurve(const xt::xtensor<float, 2>& src, const Curve& curve, float offset = 0.0f);
			xt::xtensor<float, 2> calcAccumIntegralOverCurve(const xt::xtensor<float, 2>& src, const Curve& curve, const xt::xtensor<float, 1>& offsets);
		}

		struct CurveGenerationOptions
		{
			float max_angle = 1.0f; //in radians
			float rel_step_size = 0.05f; //in percent of image width
			float rel_window_size_x = 0.15f; //in percent of image width
			float rel_window_size_y = 0.15f; //in percent of image height
			unsigned num_radon_angles = 51; //odd to include zero
			unsigned num_radon_offsets = 50;
			float angle_regularization_coef = 0.1f;
			float angle_regularization_power = 0.25f;
		};

		
		std::vector<std::tuple<unsigned, unsigned>> selectSupportPoints(const xt::xtensor<float, 2>& src,
			unsigned num_points, float quantile_v, float quantile_h);

		Curve generateCurve(const xt::xtensor<float, 2>& src, int i0, int j0, const CurveGenerationOptions& options);
		Curve optimizeCurve(const xt::xtensor<float, 2>& src, const Curve& curve, int max_offset_y, int max_window_x, float reg_coef);
		Curve reparamByLength(const Curve& curve, unsigned numPoints);
		Curve reparamByX(const Curve& curve, unsigned numPoints);
	}
}