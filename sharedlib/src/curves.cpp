#include "curves.h"
#include "extremum.h"
#include "radon.h"
#include <xtensor/xview.hpp>
#include <xtensor/xsort.hpp>

using namespace bujo::curves;

std::vector<std::tuple<unsigned, unsigned>> bujo::curves::selectSupportPoints(const xt::xtensor<float, 2>& src, unsigned num_points, float quantile_v, float quantile_h)
{
	constexpr int iwidth = 1;
	auto h_rng = bujo::extremum::getSuperRangeQuantile(xt::mean(src, 0), quantile_h);
	auto v_rng = bujo::extremum::getSuperRangeQuantile(xt::mean(src, 1), quantile_v);

	auto ispace = xt::linspace(std::get<0>(v_rng), std::get<1>(v_rng), num_points);
	auto jspace = xt::linspace(std::get<0>(h_rng), std::get<1>(h_rng), num_points);
	auto vmean = xt::mean(src, 1);

	int dx = static_cast<int>(std::floorf((jspace.at(1) - jspace.at(0)) * 0.25f));
	int dy = static_cast<int>(std::floorf((ispace.at(1) - ispace.at(0)) * 0.25f));

	std::vector<std::tuple<unsigned, unsigned>> res;
	res.reserve(num_points);

	for (int i = 0; i < num_points; i++)
	{
		int real_i = static_cast<int>(ispace.at(i));
		int i_lo = std::max(0, real_i - dy);
		int i_hi = std::min(static_cast<int>(src.shape()[0]), real_i + dy);
		int iidx = xt::argmax(xt::view(vmean, xt::range(i_lo, i_hi)))[0];
		int best_i = iidx + i_lo;

		int bi_lo = std::max(0, best_i - iwidth);
		int bi_hi = std::min(static_cast<int>(src.shape()[0]), best_i + iwidth + 1);

		
		int best_j = -1;
		float best_v = -1.0f;
		for (int j = 0; j < num_points; j++)
		{
			int real_j = static_cast<int>(jspace.at(j));
			int j_lo = std::max(0, real_j - dx);
			int j_hi = std::min(static_cast<int>(src.shape()[1]), real_j + dx);
			float jreg = 2.0f * (0.5f - std::fabsf(0.5f - static_cast<float>(j)/static_cast<float>(num_points))) + 0.2f;

			float val0 = xt::mean(xt::view(src, xt::range(bi_lo, bi_hi), xt::range(j_lo, j_hi)))[0];
			float val = val0 * jreg;
			if (val > best_v)
			{
				best_j = real_j;
				best_v = val;
			}
		}
		res.emplace_back(unsigned(best_i), unsigned(best_j));
	}

	return res;
}

/*
Performs checks on arguments, creates specified views and returns result. Just fancy wrapper.
*/
float getLocalAngle_(const xt::xtensor<float, 2>& src, const xt::xtensor<float, 1>& angles, int i0, int i1, int j0, int j1,
	const CurveGenerationOptions &options)
{
	int ri0 = std::max(0, std::min(i0, i1));
	int ri1 = std::min(static_cast<int>(src.shape()[0]), std::max(i0, i1));

	int rj0 = std::max(0, std::min(j0, j1));
	int rj1 = std::min(static_cast<int>(src.shape()[0]), std::max(j0, j1));

	return bujo::radon::findAngle(xt::view(src, xt::range(ri0, ri1), xt::range(rj0, rj1)),
		angles, options.num_radon_offsets, bujo::radon::RT_RADON,
		options.angle_regularization_coef, options.angle_regularization_power);
}

/*
Calculates curve from starting position (i0, j0) with directed step. Each step is in direction of regularized local angle.

Keyword arguments:
	src -- source image
	i0, j0 -- starting position
	dir -- direction of search
*/

std::vector<std::tuple<int, int>> makeCurve1_(const xt::xtensor<float, 2>& src, int i0, int j0, int dir,
	const CurveGenerationOptions& options)
{
	
	int window_size_x = static_cast<int>(std::ceilf(src.shape()[1] * options.rel_window_size_x));
	int window_size_y = static_cast<int>(std::ceilf(src.shape()[0] * options.rel_window_size_y));
	int di_neg = window_size_y / 2;
	int di_pos = window_size_y - di_neg;
	int dj_neg = window_size_x / 2;
	int dj_pos = window_size_x - dj_neg;

	auto angles = xt::linspace(-options.max_angle, options.max_angle, options.num_radon_angles);
	float step = dir * (options.rel_step_size * src.shape()[1]);
	
	std::vector<std::tuple<int, int>> res;
	res.reserve(static_cast<size_t>(std::ceilf(1.5f / options.rel_step_size)));
	res.emplace_back(i0, j0);
	
	while (1)
	{
		int ci = std::get<0>(res.back());
		int cj = std::get<1>(res.back());
		//step like Runge-Kutta, i.e. we take 0.5 step in local angle, calculate new angle and take full step in that direction
		float angle0 = getLocalAngle_(src, angles, ci-di_neg, ci+di_pos, cj-dj_neg, cj+dj_pos, options);
		float di0 = step * std::sinf(angle0) * 0.5f;
		float dj0 = step * std::cosf(angle0) * 0.5f;
		int ni0 = static_cast<int>(std::roundf(ci + di0));
		int nj0 = static_cast<int>(std::roundf(cj + dj0));

		float angle1 = getLocalAngle_(src, angles, ni0 - di_neg, ni0 + di_pos, nj0 - dj_neg, nj0 + dj_pos, options);
		float di1 = step * std::sinf(angle1);
		float dj1 = step * std::cosf(angle1);
		int ni1 = static_cast<int>(std::roundf(ci + di1));
		int nj1 = static_cast<int>(std::roundf(cj + dj1));
		
		if ((ni1 < 0) || (ni1 >= src.shape()[0]) || (nj1 < 0) || (nj1 >= src.shape()[1]))
		{
			int ri = std::max(0, std::min(static_cast<int>(src.shape()[0] - 1), ni1));
			int rj = std::max(0, std::min(static_cast<int>(src.shape()[1] - 1), nj1));
			float rdi = 1.0f, rdj = 1.0f;
			
			if (std::fabsf(di1) > 1e-5)
				rdi = float(ri - ci) / di1;
			if (std::fabsf(dj1) > 1e-5)
				rdj = float(rj - cj) / dj1;

			float rstep = std::min(rdi, rdj) * 0.9f;

			ni1 = static_cast<int>(std::roundf(ci + di1 * rstep));
			nj1 = static_cast<int>(std::roundf(cj + dj1 * rstep));
			res.emplace_back(ni1, nj1);
			break;
		}
		else
			res.emplace_back(ni1, nj1);
	}
	return res;
}

std::vector<std::tuple<int, int>> makeCurve2_(const xt::xtensor<float, 2>& src, int i0, int j0,
	const CurveGenerationOptions& options)
{
	auto crv_p = std::move(makeCurve1_(src, i0, j0, 1, options));
	auto crv_n = std::move(makeCurve1_(src, i0, j0, -1, options));
	int crv_neg_size = crv_n.size() - 1;
	std::vector<std::tuple<int, int>> res(crv_p.size() + crv_neg_size);
	for (int i = 0; i < crv_neg_size; i++)
		res[i] = crv_n[crv_neg_size - i];
	for (int i = 0; i < crv_p.size(); i++)
		res[crv_neg_size + i] = crv_p[i];
	return res;
}


Curve bujo::curves::generateCurve(const xt::xtensor<float, 2>& src, int i0, int j0, const CurveGenerationOptions &options)
{
	auto crv0 = makeCurve2_(src, i0, j0, options);
	float length = 0.0f;
	Curve res;
	res.len_param.resize({ crv0.size() });
	res.x_value.resize({ crv0.size() });
	res.y_value.resize({ crv0.size() });
	
	for (int i = 0; i < crv0.size(); i++)
	{
		res.y_value[i] = std::get<0>(crv0[i]);
		res.x_value[i] = std::get<1>(crv0[i]);
		if (i > 0)
		{
			float dx = res.x_value[i] - res.x_value[i - 1];
			float dy = res.y_value[i] - res.y_value[i - 1];
			float dl = std::sqrtf(dx * dx + dy * dy);
			length += dl;
		}
		res.len_param[i] = length;
	}
	res.len_param /= length;
	return res;
}

struct RecursiveSegmentInfo
{
	int i0, i1, offset;
	float integral_value;
	RecursiveSegmentInfo* pLeft = nullptr;
	RecursiveSegmentInfo* pRight = nullptr;
};

void clearRecursive_(RecursiveSegmentInfo* segInfo)
{
	if (!segInfo)
		return;
	clearRecursive_(segInfo->pLeft);
	clearRecursive_(segInfo->pRight);
	delete segInfo;
}

RecursiveSegmentInfo* buildRecursiveOffsets_(const xt::xtensor<float, 2>& arr2d, int i0, int i1, int offset, int min_window)
{
	float integral_value = xt::view(arr2d, -1, offset)[0] - xt::view(arr2d, 0, offset)[0];

	RecursiveSegmentInfo* res = new RecursiveSegmentInfo;
	res->i0 = i0;
	res->i1 = i1;
	res->integral_value = integral_value;
	res->offset = offset;
	res->pLeft = nullptr;
	res->pRight = nullptr;

	if ((arr2d.shape()[0] < min_window) || (integral_value <= 1e-7))
		return res;

	size_t midpoint = arr2d.shape()[0] / 2;
	auto arr_l = xt::view(arr2d, midpoint, xt::all()) - xt::view(arr2d, 0, xt::all());
	auto arr_r = xt::view(arr2d, -1, xt::all()) - xt::view(arr2d, midpoint, xt::all());
	/*
	midpoint = arr2d.shape[0] // 2
	arr_l = arr2d[midpoint,:] - arr2d[0,:]
	arr_r = arr2d[-1,:] - arr2d[midpoint,:]
	off_l = find_local_maximum_by_ascend(arr_l, offset)
	off_r = find_local_maximum_by_ascend(arr_r, offset)
	return (i0, i1, offset, integral_value,
			calc_recursive_offsets_by_midpoint(arr2d[:midpoint], i0, i0+midpoint, off_l, min_window),
			calc_recursive_offsets_by_midpoint(arr2d[midpoint:], i0+midpoint, i1, off_r, min_window))

	*/
}

Curve bujo::curves::optimizeCurve(const xt::xtensor<float, 2>& src, const Curve& curve, int max_offset_y, int max_window_x)
{
	int num_offsets = max_offset_y * 2 + 1;
	xt::xtensor<float, 1> offsets;
	offsets.resize({ static_cast<size_t>(num_offsets) });
	for (int i = 0; i < num_offsets; i++)
		offsets[i] = i - max_offset_y;

	xt::xtensor<float, 2> cumulativeIntegral = integral::calcAccumIntegralOverCurve(src, curve, offsets);
	
	Curve res;

	return res;
}

float bujo::curves::integral::calcIntegralOverCurve(const xt::xtensor<float, 2>& src, const Curve& curve, float offset)
{
	xt::xtensor<float, 1> off_tensor;
	off_tensor.resize({ 1 });
	off_tensor[0] = offset;
	return integral::calcIntegralOverCurve(src, curve, off_tensor)[0];
}

xt::xtensor<float, 1> bujo::curves::integral::calcIntegralOverCurve(const xt::xtensor<float, 2>& src, const Curve& curve, const xt::xtensor<float, 1>& offsets)
{
	xt::xtensor<float, 1> res;
	res.resize({ offsets.size() });
	auto xyLocs = interpolate::getDenseXY(curve);
	const auto& xLocs = std::get<0>(xyLocs);
	const auto& yLocs = std::get<1>(xyLocs);
	for (int i = 0; i < offsets.size(); i++)
	{
		float val = 0.0f;
		for (int j = 0; j < xLocs.size(); j++)
		{
			int real_j = static_cast<int>(std::roundf(xLocs[j]));
			int real_i = static_cast<int>(std::roundf(yLocs[j] + offsets[i]));
			val += src.at(real_i, real_j);
		}
		res[i] = val;
	}
	return std::move(res);
}

xt::xtensor<float, 1> bujo::curves::integral::calcAccumIntegralOverCurve(const xt::xtensor<float, 2>& src, const Curve& curve, float offset)
{
	xt::xtensor<float, 1> off_tensor;
	off_tensor.resize({ 1 });
	off_tensor[0] = offset;
	return xt::view(integral::calcAccumIntegralOverCurve(src, curve, off_tensor), xt::all(), 0);
}

xt::xtensor<float, 2> bujo::curves::integral::calcAccumIntegralOverCurve(const xt::xtensor<float, 2>& src, const Curve& curve, const xt::xtensor<float, 1>& offsets)
{
	auto xyLocs = interpolate::getDenseXY(curve);
	const auto& xLocs = std::get<0>(xyLocs);
	const auto& yLocs = std::get<1>(xyLocs);

	xt::xtensor<float, 2> res({ xLocs.size(), offsets.size() });
	
	for (int i = 0; i < offsets.size(); i++)
	{
		float val = 0.0f;
		for (int j = 0; j < xLocs.size(); j++)
		{
			int real_j = static_cast<int>(std::roundf(xLocs[j]));
			int real_i = static_cast<int>(std::roundf(yLocs[j] + offsets[i]));
			val += src.at(real_i, real_j);
			res.at(j, i) = val;
		}
	}
	return std::move(res);
}


std::tuple<xt::xtensor<float, 1>, xt::xtensor<float, 1>> bujo::curves::interpolate::getDenseXY(const Curve& curve)
{
	float xMin = xt::amin(curve.x_value)[0], xMax = xt::amax(curve.x_value)[0];
	int jMin = static_cast<int>(std::floorf(xMin));
	int jMax = static_cast<int>(std::ceilf(xMax));
	int numPoints = jMax - jMin + 1;

	xt::xtensor<float, 1> resX;
	resX.resize({ static_cast<size_t>(numPoints) });
	for (int j = 0; j < numPoints; j++)
		resX[j] = jMin + j;

	xt::xtensor<float, 1> resY = xt::interp(resX, curve.x_value, curve.y_value);

	return std::make_tuple(std::move(resX), std::move(resY));
}
