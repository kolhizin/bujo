#pragma once
#include <opencv2\opencv.hpp>

namespace bujo
{
	namespace transform
	{
		float getTextAngle(const cv::Mat &src);
		int getTextLineDelta(const cv::Mat& src);
		cv::Mat rotateImage(const cv::Mat& src, float angle);

		cv::Mat applyVarianceCutoff(const cv::Mat& src, unsigned size_w, unsigned size_h, double q);
	}
}