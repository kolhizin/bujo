#pragma once
#include <opencv2\opencv.hpp>

namespace bujo
{
	namespace radon
	{
		void createGrid(cv::Mat &dst, const cv::Mat &src, const cv::Mat &angles);
	}
}