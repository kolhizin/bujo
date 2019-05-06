#pragma once
#include <xtensor/xtensor.hpp>

namespace bujo
{
	namespace radon
	{
		enum TransformType {RT_RADON_VANILLA, RT_RADON, RT_HOUGH, RT_RADON_ACCURATE};

		std::tuple<xt::xtensor<float, 2>, xt::xtensor<float, 1>> radon(const xt::xtensor<float, 2>&src, const xt::xtensor<float, 1> &angles,
								unsigned num_offset, unsigned transformType);

		float findAngle(const xt::xtensor<float, 2>& src, const xt::xtensor<float, 1>& angles,
			unsigned num_offset, unsigned transformType, float reg_coef=0.1f, float reg_power=0.25f);
	}
}