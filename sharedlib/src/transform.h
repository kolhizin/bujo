#pragma once
#include <opencv2\opencv.hpp>

namespace bujo
{
	namespace transform
	{
		float getTextAngle(const cv::Mat &src);
		int getTextLineDelta(const cv::Mat& src);
		cv::Mat rotateImage(const cv::Mat& src, float angle);

		cv::Mat applyVarianceCutoff(const cv::Mat& src, unsigned size_w, unsigned size_h,
			float cutoff_q = 0.9, float cutoff_coef = 0.5);
	
		cv::Mat coarseImage(const cv::Mat& src, float scale = 0.25f, float sigma = 0.2f, float gaussian_threshold = 0.4f);
	}
}