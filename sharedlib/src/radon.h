#pragma once
#include <opencv2\opencv.hpp>

namespace bujo
{
	namespace radon
	{
		enum TransformType {RT_RADON, RT_HOUGH};

		void radon(cv::Mat& dst, const cv::Mat& src, const cv::Mat& angles, unsigned int num_offset,
			unsigned transformType);
	}
}