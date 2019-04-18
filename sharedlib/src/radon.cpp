#include "radon.h"
#include <exception>
using namespace bujo::radon;

void impl_vanila_radon_(cv::Mat& dst, const cv::Mat& src, const cv::Mat& angles, const cv::Mat& offsets)
{
	int numAngles = angles.size[0];
	for (int i = 0; i < numAngles; i++)
		for (int j = 0; j < num_offset; j++)
		{
			int m_min = , m_max = ;
		}

}

void radon(cv::Mat& dst, const cv::Mat& src, const cv::Mat& angles, unsigned int num_offset, unsigned transformType)
{
	if (angles.size.dims() != 1)
		throw std::logic_error("Function radon() expects angles to be 1d-array, got otherwise!");
	if (src.size.dims() != 2)
		throw std::logic_error("Function radon() expects source-image (src) to be 2d-array, got otherwise!");
	dst = cv::Mat::zeros(angles.size[0], num_offset); //type?
	if (transformType == TransformType::RT_RADON)
		impl_vanila_radon_(dst, src, angles, num_offset);
	else if(transformType == TransformType::RT_HOUGH)

}