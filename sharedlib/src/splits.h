#pragma once
#include <xtensor/xtensor.hpp>
#include <functional>
#include <optional>

namespace bujo
{
	namespace splits
	{
		struct SplitStat
		{
			float volume_inside, volume_before, volume_after;
			float margin_before, margin_after;
		};
		struct RegionSplit
		{
			float angle, offset;

			SplitStat stats;
		};

		std::optional<RegionSplit> findBestVSplit(const xt::xtensor<float, 2>& src, const xt::xtensor<float, 1>& angles,
			unsigned num_offsets, unsigned window_size, float minimal_abs_split_intensity, float maximal_abs_intersection);
	}
}
