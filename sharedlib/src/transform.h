#pragma once
#include <xtensor/xtensor.hpp>

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
	}
}