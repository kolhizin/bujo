#include "radon.h"
#include <exception>

using namespace bujo::radon;

xt::xarray<float> set_uniform_offset_range_(const xt::xarray<float>& src, unsigned num_offsets)
{
	float sz = std::max(src.shape()[0], src.shape()[1]);
	float diff = sz / num_offsets;
	return xt::linspace(-sz + diff, sz - diff, num_offsets);
}

std::tuple<unsigned, unsigned> get_line_range_(float off, float slope, unsigned w, unsigned h)
{

}

std::tuple<xt::xarray<float>, xt::xarray<float>>
	impl_vanila_radon_(const xt::xarray<float>& src, const xt::xarray<float>& angles,unsigned num_offsets)
{
	unsigned num_angles = angles.size();
	auto offsets = set_uniform_offset_range_(src, num_offsets);
	float x_cutoff = 1.0f / std::sqrtf(2.0f);

	for (int i = 0; i < num_angles; i++)
	{
		float a_cos = std::cosf(angles[i]), a_sin = std::sinf(angles[i]);
		if (std::fabsf(a_sin) > x_cutoff)
		{
			float slope = -a_cos / a_sin;
			for (int j = 0; j < num_offsets; j++)
			{
				float off = offsets[j] / a_sin;
			}
		}
		else
		{
			float slope = -a_sin / a_cos;
			for (int j = 0; j < num_offsets; j++)
			{
				float off = offsets[j] / a_cos;
			}
		}
	}

}
/*

void impl_vanila_hough_(cv::Mat& dst, cv::Mat& offsets, const cv::Mat& src, const cv::Mat& angles, unsigned num_offsets)
{

}

void radon(cv::Mat& dst, cv::Mat& offsets, const cv::Mat& src, const cv::Mat& angles, unsigned int num_offset, unsigned transformType)
{
	if (angles.size.dims() != 1)
		throw std::logic_error("Function radon() expects angles to be 1d-array, got otherwise!");
	if (src.size.dims() != 2)
		throw std::logic_error("Function radon() expects source-image (src) to be 2d-array, got otherwise!");

	dst = cv::Mat::zeros(angles.size[0], num_offset, CV_32F); 

	if (transformType == TransformType::RT_RADON)
		impl_vanila_radon_(dst, offsets, src, angles, num_offset);
	else if (transformType == TransformType::RT_HOUGH)
		impl_vanila_hough_(dst, offsets, src, angles, num_offset);
	else throw std::logic_error("Function radon() got unexpected transformType argument!");
}
*/
std::tuple<xt::xarray<float>, xt::xarray<float>> radon(const xt::xarray<float>& src, const xt::xarray<float>& angles, unsigned num_offset, unsigned transformType)
{
	if (angles.shape().size() != 1)
		throw std::logic_error("Function radon() expects angles to be 1d-array, got otherwise!");
	if (src.shape().size() != 2)
		throw std::logic_error("Function radon() expects src to be 2d-array, got otherwise!");

	if (transformType == TransformType::RT_RADON)
		return impl_vanila_radon_(src, angles, num_offset);
	//else if (transformType == TransformType::RT_HOUGH)
	//	return impl_vanila_hough_(src, angles, num_offset);
	else throw std::logic_error("Function radon() got unexpected transformType argument!");
}
