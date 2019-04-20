#include "radon.h"
#include <exception>

using namespace bujo::radon;

xt::xarray<float> set_uniform_offset_range_(const xt::xarray<float>& src, unsigned num_offsets)
{
	float sz = std::sqrtf(src.shape()[0]*src.shape()[0] + src.shape()[1]*src.shape()[1]) * 0.5f;
	float diff = sz / num_offsets;
	return xt::linspace(-sz - diff, sz + diff, num_offsets);
}

std::tuple<int, int> get_line_range_(float off, float slope, unsigned s0, unsigned s1)
{
	if(slope == 0.0f) //exact test with zero is alright
		throw std::logic_error("Function get_line_range_() expects non-zero slope!");

	float f1 = -(off + 0.5f) / slope;
	float f2 = (s1 - 0.5f - off) / slope;
	if (slope > 0.0f)
	{
		int rmin = std::max(int(0), int(std::ceilf(f1)));
		int rmax = std::min(int(s0) - 1, int(std::floorf(f2)));
		return std::make_tuple(rmin, rmax);
	}else{
		int rmin = std::max(int(0), int(std::ceilf(f2)));
		int rmax = std::min(int(s0) - 1, int(std::floorf(f1)));
		return std::make_tuple(rmin, rmax);
	}
}


std::tuple<xt::xarray<float>, xt::xarray<float>> impl_vanilla_radon_(const xt::xarray<float>& src, const xt::xarray<float>& angles, unsigned num_offsets)
{
	unsigned num_angles = angles.size();
	xt::xarray<float> offsets = set_uniform_offset_range_(src, num_offsets);
	float x_cutoff = 1.0f / std::sqrtf(2.0f);
	xt::xarray<float> radon_res = xt::zeros<float>({ angles.size(), offsets.size() });

	float x_min = -float(src.shape()[1]) * 0.5f;
	float y_min = -float(src.shape()[0]) * 0.5f;

	for (int i = 0; i < num_angles; i++)
	{
		float a_cos = std::cosf(angles[i]), a_sin = std::sinf(angles[i]);
		float r_offset = x_min * a_cos + y_min * a_sin;
		if (std::fabsf(a_sin) > x_cutoff)
		{
			float slope = -a_cos / a_sin;
			for (int j = 0; j < num_offsets; j++)
			{
				float off = (offsets[j] - r_offset) / a_sin;
				float res = 0.0f;
				for (int x = 0; x < src.shape()[1]; x++)
				{
					int y = int(std::roundf(off + slope * x));
					if ((y < 0) || (y >= src.shape()[0]))
						continue;
					res += src(y, x);
				}
				radon_res(i, j) = res;
			}
		}
		else
		{
			float slope = -a_sin / a_cos;
			for (int j = 0; j < num_offsets; j++)
			{
				float off = (offsets[j] - r_offset) / a_cos;
				float res = 0.0f;
				for (int y = 0; y < src.shape()[0]; y++)
				{
					int x = int(std::roundf(off + slope * y));
					if ((x < 0) || (x >= src.shape()[1]))
						continue;
					res += src(y, x);
				}
				radon_res(i, j) = res;
			}
		}
	}
	return std::make_tuple(radon_res, offsets);
}


std::tuple<xt::xarray<float>, xt::xarray<float>> impl_optimized_radon_(const xt::xarray<float>& src, const xt::xarray<float>& angles,unsigned num_offsets)
{
	unsigned num_angles = angles.size();
	xt::xarray<float> offsets = set_uniform_offset_range_(src, num_offsets);
	float x_cutoff = 1.0f / std::sqrtf(2.0f);
	xt::xarray<float> radon_res = xt::zeros<float>({ angles.size(), offsets.size() });

	float x_min = -float(src.shape()[1]) * 0.5f;
	float y_min = -float(src.shape()[0]) * 0.5f;

	for (int i = 0; i < num_angles; i++)
	{
		float a_cos = std::cosf(angles[i]), a_sin = std::sinf(angles[i]);
		float r_offset = x_min * a_cos + y_min * a_sin;
		if (std::fabsf(a_sin) > x_cutoff)
		{
			float slope = -a_cos / a_sin;
			for (int j = 0; j < num_offsets; j++)
			{
				float off = (offsets[j] - r_offset) / a_sin;
				auto xrange = get_line_range_(off, slope, src.shape()[1], src.shape()[0]);
				float res = 0.0f;
				for (int x = std::get<0>(xrange); x <= std::get<1>(xrange); x++)
				{
					int y = int(std::roundf(off + slope * x));
					if ((y < 0) || (y >= src.shape()[0]))
						continue;
					res += src(y, x);
				}
				radon_res(i, j) = res;
			}
		}
		else
		{
			float slope = -a_sin / a_cos;
			for (int j = 0; j < num_offsets; j++)
			{
				float off = (offsets[j] - r_offset) / a_cos;
				auto yrange = get_line_range_(off, slope, src.shape()[0], src.shape()[1]);
				float res = 0.0f;
				for (int y = std::get<0>(yrange); y <= std::get<1>(yrange); y++)
				{
					int x = int(std::roundf(off + slope * y));
					if ((x < 0) || (x >= src.shape()[1]))
						continue;
					res += src(y, x);
				}
				radon_res(i, j) = res;
				
			}
		}
	}
	return std::make_tuple(radon_res, offsets);
}
/*

void impl_vanila_hough_(cv::Mat& dst, cv::Mat& offsets, const cv::Mat& src, const cv::Mat& angles, unsigned num_offsets)
{

}
*/
std::tuple<xt::xarray<float>, xt::xarray<float>> bujo::radon::radon(const xt::xarray<float>& src, const xt::xarray<float>& angles, unsigned num_offset, unsigned transformType)
{
	if (angles.shape().size() != 1)
		throw std::logic_error("Function radon() expects angles to be 1d-array, got otherwise!");
	if (src.shape().size() != 2)
		throw std::logic_error("Function radon() expects src to be 2d-array, got otherwise!");

	if (transformType == TransformType::RT_RADON_VANILLA)
		return impl_vanilla_radon_(src, angles, num_offset);
	else if (transformType == TransformType::RT_RADON)
		return impl_optimized_radon_(src, angles, num_offset);
	//else if (transformType == TransformType::RT_HOUGH)
	//	return impl_vanila_hough_(src, angles, num_offset);
	else throw std::logic_error("Function radon() got unexpected transformType argument!");
}
