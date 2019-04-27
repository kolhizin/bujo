#pragma once
#include <vector>
#include <xtensor/xtensor.hpp>
#include "splits.h"

namespace bujo
{
	namespace transform
	{
		xt::xtensor<float, 2> resizeImage(const xt::xtensor<float, 2>& src, float factor);
		xt::xtensor<float, 2> rotateImage(const xt::xtensor<float, 2>& src, float angle);
		
		float getTextAngle(const xt::xtensor<float, 2> &src);
		int getTextLineDelta(const xt::xtensor<float, 2>& src);

		xt::xtensor<float, 2> filterVarianceQuantile(const xt::xtensor<float, 2>& src, unsigned size_w, unsigned size_h, float qw, float qh);
		float calculateQuantile(const xt::xtensor<float, 2>& src, float quantile);
		xt::xtensor<float, 2> thresholdImage(const xt::xtensor<float, 2>& src, float cutoff);
		xt::xtensor<float, 2> coarseImage(const xt::xtensor<float, 2>& src, float scale, float sigma, float cutoff);

		std::vector<bujo::splits::RegionSplit> findVSplits(const xt::xtensor<float, 2>& src, float min_angle, unsigned num_angles,
			float minimal_abs_split_intensity, float maximal_abs_intersection, float minimal_pct_split);
		void setRegionsValue(xt::xtensor<float, 2>& src, std::vector<bujo::splits::RegionSplit> splits, float dsize, float value);
	}
}