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
	}
}