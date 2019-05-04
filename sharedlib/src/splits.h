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

		template<class T, class Fn>
		inline void transformRegion(T& src, const SplitDesc& split, int direction, Fn f)
		{
			float x_min = -float(src.shape()[1]) * 0.5f;
			float y_min = -float(src.shape()[0]) * 0.5f;
			float a_x = std::sinf(split.angle), a_y = -std::cosf(split.angle);
			float r_offset = x_min * a_x + y_min * a_y;
			int f_dir = split.direction * direction;

			for (int i = 0; i < src.shape()[0]; i++)
				for (int j = 0; j < src.shape()[1]; j++)
				{
					float val = (i * a_y + j * a_x - split.offset + r_offset);
					if (std::roundf(val * f_dir) >= 0.0f)
						f(src.at(i, j));
				}
		}

		void setRegionValue(xt::xtensor<float, 2>& src, const SplitDesc& split, int direction, float value);
		float calcRegionQuantile(const xt::xtensor<float, 2>& src, const SplitDesc& split, int direction, float quantile);
		std::tuple<float, float> calcRegionMeanStd(const xt::xtensor<float, 2>& src, const SplitDesc& split, int direction);
		inline float calcRegionMean(const xt::xtensor<float, 2>& src, const SplitDesc& split, int direction)
		{
			return std::get<0>(calcRegionMeanStd(src, split, direction));
		}
		inline float calcRegionStd(const xt::xtensor<float, 2>& src, const SplitDesc& split, int direction)
		{
			return std::get<1>(calcRegionMeanStd(src, split, direction));
		}
		SplitDesc rescaleSplit(const SplitDesc& desc, float dsize);
	}
}
