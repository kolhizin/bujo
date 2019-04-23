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
		
		/*
		cv::Mat applyVarianceCutoff(const cv::Mat& src, unsigned size_w, unsigned size_h,
			float cutoff_q = 0.9, float cutoff_coef = 0.5);
	
		cv::Mat coarseImage(const cv::Mat& src, float scale = 0.25f, float sigma = 0.2f, float gaussian_threshold = 0.4f);
		*/
	}
}