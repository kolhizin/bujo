#pragma once
#include <xtensor/xtensor.hpp>

namespace bujo
{
	namespace filters
	{
		xt::xtensor<float, 2> filterVariance2DV(const xt::xtensor<float, 2>& src, unsigned size);
		xt::xtensor<float, 2> filterVariance2DH(const xt::xtensor<float, 2>& src, unsigned size);
		xt::xtensor<float, 2> filterQuantile2D(const xt::xtensor<float, 2>& src, unsigned size_w, unsigned size_h, float quantile);
		
		xt::xtensor<float, 2> filterVarianceQuantileV(const xt::xtensor<float, 2>& src, unsigned size_w, unsigned size_h, float quantile);
		xt::xtensor<float, 2> filterVarianceQuantileH(const xt::xtensor<float, 2>& src, unsigned size_w, unsigned size_h, float quantile);
		xt::xtensor<float, 2> filterVarianceQuantileVH(const xt::xtensor<float, 2>& src, unsigned size_w, unsigned size_h, float quantile_w, float quantile_h);

		xt::xtensor<float, 2> filterGaussian2D(const xt::xtensor<float, 2>& src, float sigma);

		xt::xtensor<float, 2> filterLocalMax2DV(const xt::xtensor<float, 2>& src,
			unsigned window_size, unsigned slack_size, float threshold);

		xt::xtensor<float, 1> filterMax1D(const xt::xtensor<float, 1>& src, unsigned window);
		xt::xtensor<float, 1> filterMin1D(const xt::xtensor<float, 1>& src, unsigned window);
		xt::xtensor<float, 1> filterMax1D1Way(const xt::xtensor<float, 1>& src, int window);
		xt::xtensor<float, 1> filterMin1D1Way(const xt::xtensor<float, 1>& src, int window);
	}
}