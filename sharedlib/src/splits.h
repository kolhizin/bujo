#pragma once
#include <xtensor/xtensor.hpp>

namespace bujo
{
	namespace splits
	{
		struct SplitStat
		{
			float volume_inside, volume_before, volume_after;
			float margin_before, margin_after;
		};
		struct SplitDesc
		{
			float angle, offset;
			int direction;
		};
		struct RegionSplit
		{
			SplitDesc desc;
			SplitStat stats;
		};

		RegionSplit findBestVSplit(const xt::xtensor<float, 2>& src, const xt::xtensor<float, 1>& angles,
			unsigned num_offsets, unsigned window_size, float minimal_abs_split_intensity, float maximal_abs_intersection, float minimal_pct_split);

		void updateRegion(xt::xtensor<float, 2>& src, const SplitDesc& split, float value, int direction);
	}
}
