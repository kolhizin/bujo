#pragma once
#include <xtensor/xarray.hpp>

namespace bujo
{
	namespace filters
	{
		xt::xarray<float> filterVariance2DV(const xt::xarray<float>& src, unsigned size);
		xt::xarray<float> filterVariance2DH(const xt::xarray<float>& src, unsigned size);
		xt::xarray<float> filterQuantile2D(const xt::xarray<float>& src, unsigned size_w, unsigned size_h, float quantile);
		
		xt::xarray<float> filterVarianceQuantileV(const xt::xarray<float>& src, unsigned size_w, unsigned size_h, float quantile);
		xt::xarray<float> filterVarianceQuantileH(const xt::xarray<float>& src, unsigned size_w, unsigned size_h, float quantile);
		xt::xarray<float> filterVarianceQuantileVH(const xt::xarray<float>& src, unsigned size_w, unsigned size_h, float quantile_v, float quantile_h);
	}
}