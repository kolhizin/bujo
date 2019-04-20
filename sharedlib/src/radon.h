#pragma once
#include <xtensor/xarray.hpp>

namespace bujo
{
	namespace radon
	{
		enum TransformType {RT_RADON_VANILLA, RT_RADON, RT_HOUGH};

		std::tuple<xt::xarray<float>, xt::xarray<float>> radon(const xt::xarray<float> &src, const xt::xarray<float> &angles,
								unsigned num_offset, unsigned transformType);
	}
}