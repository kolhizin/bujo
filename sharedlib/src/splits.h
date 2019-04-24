#pragma once
#include <xtensor/xtensor.hpp>

namespace bujo
{
	namespace splits
	{
		struct RegionSplit
		{
			float angle, offset;

			float volume_before;
			float volume_inside;
			float volume_after;
		};

		RegionSplit findBestVSplit(const xt::xtensor<float, 2>& src, const xt::xtensor<float, 1>& angles);
	}
}
