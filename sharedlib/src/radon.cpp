#include "radon.h"
#include <exception>
#include <xtensor/xsort.hpp>

using namespace bujo::radon;

xt::xtensor<float, 1> set_uniform_offset_range_(const xt::xtensor<float, 2>& src, unsigned num_offsets)
{
	float sz = std::sqrtf(static_cast<float>(src.shape()[0]*src.shape()[0] + src.shape()[1]*src.shape()[1])) * 0.5f;
	float diff = sz / num_offsets;
	return xt::linspace(-sz - diff, sz + diff, num_offsets);
}

std::tuple<int, int> get_line_range_(float off, float slope, unsigned s0, unsigned s1)
{
	if (std::fabsf(slope) < 1e-7f)
		return std::make_tuple(0, int(s0) - 1);

	float f1 = -(off + 0.5f) / slope;
	float f2 = (s1 - 0.5f - off) / slope;
	if (slope >= 0.0f)
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


std::tuple<xt::xtensor<float, 2>, xt::xtensor<float, 1>> impl_vanilla_radon_(const xt::xtensor<float, 2>& src, const xt::xtensor<float, 1>& angles, unsigned num_offsets)
{
	unsigned num_angles = static_cast<unsigned>(angles.size());
	xt::xtensor<float, 1> offsets = set_uniform_offset_range_(src, num_offsets);
	float a_cutoff = 1.0f / std::sqrtf(2.0f);
	xt::xtensor<float, 2> radon_res = xt::zeros<float>({ angles.size(), offsets.size() });

	float x_min = -float(src.shape()[1]) * 0.5f;
	float y_min = -float(src.shape()[0]) * 0.5f;

	for (unsigned i = 0; i < num_angles; i++)
	{
		float a_x = std::sinf(angles[i]), a_y = -std::cosf(angles[i]);
		float r_offset = x_min * a_x + y_min * a_y;
		if (std::fabsf(a_y) > a_cutoff)
		{
			float slope = -a_x / a_y;
			for (unsigned j = 0; j < num_offsets; j++)
			{
				float off = (offsets[j] - r_offset) / a_y;
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
			float slope = -a_y / a_x;
			for (unsigned j = 0; j < num_offsets; j++)
			{
				float off = (offsets[j] - r_offset) / a_x;
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


std::tuple<xt::xtensor<float, 2>, xt::xtensor<float, 1>> impl_optimized_radon_(const xt::xtensor<float, 2>& src, const xt::xtensor<float, 1>& angles,unsigned num_offsets)
{
	unsigned num_angles = static_cast<unsigned>(angles.size());
	xt::xtensor<float, 1> offsets = set_uniform_offset_range_(src, num_offsets);
	float a_cutoff = 1.0f / std::sqrtf(2.0f);
	xt::xtensor<float, 2> radon_res = xt::zeros<float>({ angles.size(), offsets.size() });

	float x_min = -float(src.shape()[1]) * 0.5f;
	float y_min = -float(src.shape()[0]) * 0.5f;

	for (unsigned i = 0; i < num_angles; i++)
	{
		float a_x = std::sinf(angles[i]), a_y = -std::cosf(angles[i]);
		float r_offset = x_min * a_x + y_min * a_y;
		if (std::fabsf(a_y) > a_cutoff)
		{
			float slope = -a_x / a_y;
			for (unsigned j = 0; j < num_offsets; j++)
			{
				float off = (offsets[j] - r_offset) / a_y;
				auto xrange = get_line_range_(off, slope, static_cast<unsigned>(src.shape()[1]), static_cast<unsigned>(src.shape()[0]));
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
			float slope = -a_y / a_x;
			for (unsigned j = 0; j < num_offsets; j++)
			{
				float off = (offsets[j] - r_offset) / a_x;
				auto yrange = get_line_range_(off, slope, static_cast<unsigned>(src.shape()[0]), static_cast<unsigned>(src.shape()[1]));
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
std::tuple<xt::xtensor<float, 2>, xt::xtensor<float, 1>> bujo::radon::radon(const xt::xtensor<float, 2>& src, const xt::xtensor<float, 1>& angles, unsigned num_offset, unsigned transformType)
{
	if (transformType == TransformType::RT_RADON_VANILLA)
		return impl_vanilla_radon_(src, angles, num_offset);
	else if (transformType == TransformType::RT_RADON)
		return impl_optimized_radon_(src, angles, num_offset);
	//else if (transformType == TransformType::RT_HOUGH)
	//	return impl_vanila_hough_(src, angles, num_offset);
	else throw std::logic_error("Function radon() got unexpected transformType argument!");
}

float bujo::radon::findAngle(const xt::xtensor<float, 2>& src, const xt::xtensor<float, 1>& angles, unsigned num_offset, unsigned transformType,
	float reg_coef, float reg_power)
{
	/*
	if np.mean(src[i0:i1,j0:j1]) < 1e-3:
		return 0
	angs = calc_local_radon_angle(grid, src, i0, i1, j0, j1)
	max_ang = np.max(np.abs(angles))
	reg_mult = np.power((reg_coef + max_ang - np.abs(angles)) / (reg_coef + max_ang), reg_power)
	return angles[np.argmax(angs * reg_mult)]
	*/
	if (xt::mean(xt::abs(src))[0] < 1e-3)
		return 0.0f;
	
	auto rtransform = std::get<0>(radon(src, angles, num_offset, transformType));
	auto x1mean = xt::mean(rtransform, 1);
	auto x2mean = xt::mean(xt::pow(rtransform, 2.0f), 1);
	auto vars = x2mean - xt::pow(x1mean, 2.0f);
	float max_angle = xt::amax(xt::abs(angles))[0];
	auto regs0 = (reg_coef + max_angle - xt::abs(angles)) / (reg_coef + max_angle);
	auto regs = xt::pow(regs0, reg_power);
	int idx = xt::argmax(regs * vars)[0];
	return angles.at(idx);
}
