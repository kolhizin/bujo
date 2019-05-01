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

xt::xtensor<float, 2> bujo::curves::extractCurveRegion(const xt::xtensor<float, 2>& src, const Curve& curve, unsigned dNeg, unsigned dPos)
{
	unsigned num_y = dPos + dNeg + 1;
	int min_x = static_cast<int>(std::floorf(xt::amin(curve.x_value)[0]));
	int max_x = static_cast<int>(std::ceilf(xt::amax(curve.x_value)[0]));
	xt::xtensor<float, 2> res({ num_y, max_x - min_x});

	xt::xtensor<float, 1> yvals = xt::interp(xt::arange<float>(max_x - min_x) + min_x, curve.x_value, curve.y_value);
	for(int i = 0; i < num_y; i++)
		for (int j = 0; j < max_x - min_x; j++)
			res.at(i, j) = src.at(yvals[j] + i - dNeg, j + min_x);
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
	}
	res.calculateLenParametrization();
	return res;
}

struct RecursiveSegmentInfo
{
	int i0, i1, offset;
	std::unique_ptr<RecursiveSegmentInfo> pLeft;
	std::unique_ptr<RecursiveSegmentInfo> pRight;
};

std::unique_ptr<RecursiveSegmentInfo> buildRecursiveOffsets_(const xt::xtensor<float, 2>& arr2d, int i0, int i1, int offset, int min_window, float reg_coef)
{
	std::unique_ptr<RecursiveSegmentInfo> res(new RecursiveSegmentInfo);
	res->i0 = i0;
	res->i1 = i1;
	res->offset = offset;

	if (arr2d.shape()[0] < min_window)
		return res;

	size_t midpoint = arr2d.shape()[0] / 2;
	auto arr_l = xt::view(arr2d, midpoint, xt::all()) - xt::view(arr2d, 0, xt::all());
	auto arr_r = xt::view(arr2d, -1, xt::all()) - xt::view(arr2d, midpoint, xt::all());

	auto reg_0 = (xt::arange<float>(0, arr2d.shape()[1]) - offset) / static_cast<float>(arr2d.shape()[1]);
	auto reg_1 = -reg_coef * xt::pow(xt::abs(reg_0), 2.0f);

	int off_l = bujo::extremum::findLocalMaximaByGradient(arr_l + reg_1, offset, true);
	int off_r = bujo::extremum::findLocalMaximaByGradient(arr_r + reg_1, offset, true);
	res->pLeft = buildRecursiveOffsets_(xt::view(arr2d, xt::range(xt::placeholders::_, midpoint), xt::all()),
		i0, i0 + midpoint, off_l, min_window, reg_coef);
	res->pRight = buildRecursiveOffsets_(xt::view(arr2d, xt::range(midpoint, xt::placeholders::_), xt::all()),
		i0 + midpoint, i1, off_r, min_window, reg_coef);
	return res;
}

void dumpRecursiveOffsetsLinear_(const std::unique_ptr<RecursiveSegmentInfo> &ptr, std::vector<std::tuple<float, float>>& res)
{
	if (!ptr)
		return;
	if ((!ptr->pLeft) && (!ptr->pRight))
	{
		res.emplace_back(static_cast<float>(ptr->i0 + ptr->i1) * 0.5f, static_cast<float>(ptr->offset));
		return;
	}
	if ((!ptr->pLeft) || (!ptr->pRight))
		throw std::runtime_error("dumpRecursiveOffsetsLinear_() incorrect combination of left & right pointers!");
	dumpRecursiveOffsetsLinear_(ptr->pLeft, res);
	dumpRecursiveOffsetsLinear_(ptr->pRight, res);
}

Curve bujo::curves::optimizeCurveBinarySplit(const xt::xtensor<float, 2>& src, const Curve& curve, int max_offset_y, int max_window_x, float reg_coef)
{
	int num_offsets = max_offset_y * 2 + 1;
	xt::xtensor<float, 1> offsets;
	offsets.resize({ static_cast<size_t>(num_offsets) });
	for (int i = 0; i < num_offsets; i++)
		offsets[i] = i - max_offset_y;

	xt::xtensor<float, 2> cumulativeIntegral = integral::calcAccumIntegralOverCurve(src, curve, offsets);

	std::vector<std::tuple<float, float>> midpoints;
	midpoints.reserve(cumulativeIntegral.shape()[0] / max_window_x);
	auto recOffsets = buildRecursiveOffsets_(cumulativeIntegral, 0, cumulativeIntegral.shape()[0], max_offset_y, max_window_x, reg_coef);
	dumpRecursiveOffsetsLinear_(recOffsets, midpoints);
	recOffsets.reset();

	xt::xtensor<float, 1> vX, vY;
	vX.resize({ midpoints.size() + 2 });
	vY.resize({ midpoints.size() + 2 });
	vX[0] = 0.0f;
	vY[0] = std::get<1>(midpoints[0]);
	for (int i = 0; i < midpoints.size(); i++)
	{
		vX[i + 1] = std::get<0>(midpoints[i]);
		vY[i + 1] = std::get<1>(midpoints[i]) - max_offset_y;
	}
	vX[vX.size() - 1] = cumulativeIntegral.shape()[0];
	vY[vY.size() - 1] = std::get<1>(midpoints.back());

	auto denseCurve = interpolate::getDenseXY(curve);
	const auto& denseX = std::get<0>(denseCurve);
	auto denseOffsets = xt::interp(denseX - xt::amin(denseX)[0], vX, vY);

	Curve res;
	res.x_value = denseX;
	res.y_value = std::get<1>(denseCurve) + denseOffsets;
	res.len_param.resize({ res.x_value.size() });
	res.calculateLenParametrization();
	return res;
}

xt::xtensor<float, 1> calcOffsetIntegrals()
{

}

Curve bujo::curves::optimizeCurve(const xt::xtensor<float, 2>& src, const Curve& curve, int max_offset_y, float reg_coef)
{
	
	return Curve();
}

float calcIntegralOverLine_(const xt::xtensor<float, 2>& arr2d, xt::xtensor<float, 1> xVals, xt::xtensor<float, 1> yVals)
{
	int min_x = static_cast<int>(std::floorf(xt::amin(xVals)[0]));
	int max_x = static_cast<int>(std::ceilf(xt::amax(xVals)[0]));
	xt::xtensor<float, 1> xs = xt::arange<float>(min_x, max_x + 1);
	xt::xtensor<float, 1> ys = xt::interp(xs, xVals, yVals);
	float res = 0.0f;
	for (int i = 0; i < xs.size(); i++)
	{
		int i0 = static_cast<int>(std::floor(ys[i]));
		float frac = ys[i] - i0;
		int i1 = i0 + 1;

		i0 = std::max(0, std::min(static_cast<int>(arr2d.shape()[0] - 1), i0));
		i1 = std::max(0, std::min(static_cast<int>(arr2d.shape()[0] - 1), i1));
		if (j + min_x >= arr2d.shape()[1])
			continue;
		float v0 = arr2d.at(i0, j + min_x);
		float v1 = arr2d.at(i1, j + min_x);
		res += v0 * (1 - frac) + v1 * frac;
	}
	return res;
}

xt::xtensor<float, 1> calcOffsetIntegrals_(const xt::xtensor<float, 2>& arr2d, xt::xtensor<float, 1> xVals, xt::xtensor<float, 1> yVals, float xNew)
{
	xt::xtensor<float, 1> res;
	res.resize({arr2d.shape()[0]});
	xt::xtensor<float, 1> xV, yV;
	xV.resize({ xVals.size() + 1 });
	yV.resize({ yVals.size() + 1 });
	int jn = 0, chng_id=0;
	for (int j = 0; j < xVals.size(); j++, jn++)
	{
		if ((xNew < xVals[j]) && (j == jn))
		{
			chng_id = jn;
			xV[jn] = xNew;
			yV[jn] = 0.0f;
			jn++;
		}
		xV[jn] = xV[j];
		yV[jn] = yV[j];
	}
	if (xVals.size() == jn)
	{
		chng_id = jn;
		xV[jn] = xNew;
		yV[jn] = 0.0f;
	}
	for (int i = 0; i < res.size(); i++)
	{
		yV[chng_id] = i;
		res[i] = calcIntegralOverLine_(arr2d, xV, yV);
	}
	return res;
}

xt::xtensor<float, 2> calcBorderIntegrals_(const xt::xtensor<float, 2>& arr2d)
{
	xt::xtensor<float, 2> res({ arr2d.shape()[0], arr2d.shape()[0] });
	xt::xtensor<float, 1> xV({ 0.0f, static_cast<float>(arr2d.shape()[1] - 1) });
	xt::xtensor<float, 1> yV({ 0.0f, 0.0f });
	for(int i = 0; i < arr2d.shape()[0]; i++)
		for (int j = 0; j < arr2d.shape()[0]; j++)
		{
			yV[0] = i;
			yV[1] = j;
			res.at(i, j) = calcIntegralOverLine_(arr2d, xV, yV);
		}
	return res;
}

xt::xtensor<float, 1> findLineStart_(const xt::xtensor<float, 2>& arr2d, float reg_coef)
{
	int offset = arr2d.shape()[0] / 2;
	xt::xtensor<float, 2> vals = calcBorderIntegrals_(arr2d);
	for(int i = 0; i < vals.shape()[0]; i++)
		for (int j = 0; j < vals.shape()[1]; j++)
		{
			int rx = i - offset;
			int ry = j - offset;
			int r = std::abs(rx + ry);
			vals.at(i, j) += reg_coef * r;
		}
	return xt::cast<float>(xt::argmax(vals));
}
/*
Curve bujo::curves::optimizeCurve2(const xt::xtensor<float, 2>& src, const Curve& curve, int max_offset_y, int max_window_x, float reg_coef)
{
	//logic: insert new point between previous optimally and then reoptimize old points
	auto offData = extractCurveRegion(src, curve, max_offset_y, max_offset_y);
	xt::xtensor<float, 1> xVals({ 0.0f, static_cast<float>(offData.shape()[1] - 1) });
	auto yVals = findLineStart_(offData, 1.0f);
	while (xVals[1] - xVals[0] > max_window_x)
	{
		for(int i = 0; i < )
	}
	return Curve();
}
*/
Curve bujo::curves::reparamByLength(const Curve& curve, unsigned numPoints)
{
	Curve res;
	auto l_param = xt::linspace<float>(0, curve.len_param.at(curve.len_param.size()-1), numPoints);
	res.x_value = xt::interp(l_param, curve.len_param, curve.x_value);
	res.y_value = xt::interp(l_param, curve.len_param, curve.y_value);
	res.calculateLenParametrization();
	return res;
}

Curve bujo::curves::reparamByX(const Curve& curve, unsigned numPoints)
{
	Curve res;
	float xmin = xt::amin(curve.x_value)[0];
	float xmax = xt::amax(curve.x_value)[0];
	auto x_param = xt::linspace<float>(xmin, xmax, numPoints);
	res.x_value = x_param;
	res.y_value = xt::interp(x_param, curve.x_value, curve.y_value);
	res.calculateLenParametrization();
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

void bujo::curves::Curve::calculateLenParametrization()
{
	float length = 0.0f;
	len_param.resize({ x_value.size() });
	for (int i = 0; i < x_value.size(); i++)
	{
		if (i > 0)
		{
			float dx = x_value[i] - x_value[i - 1];
			float dy = y_value[i] - y_value[i - 1];
			float dl = std::sqrtf(dx * dx + dy * dy);
			length += dl;
		}
		len_param[i] = length;
	}
}

