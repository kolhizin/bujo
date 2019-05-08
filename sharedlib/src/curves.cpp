#include "curves.h"
#include "extremum.h"
#include "radon.h"
#include "filters.h"
#include <xtensor/xview.hpp>
#include <xtensor/xsort.hpp>
#include <xtensor/xindex_view.hpp>
#include "util/quantiles.h"

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

	for (int i = 0; i < static_cast<int>(num_points); i++)
	{
		int real_i = static_cast<int>(ispace.at(i));
		int i_lo = std::max(0, real_i - dy);
		int i_hi = std::min(static_cast<int>(vmean.size()), real_i + dy);
		int iidx = static_cast<int>(xt::argmax(xt::view(vmean, xt::range(i_lo, i_hi)))[0]);
		int best_i = iidx + i_lo;

		int bi_lo = std::max(0, best_i - iwidth);
		int bi_hi = std::min(static_cast<int>(src.shape()[0]), best_i + iwidth + 1);

		
		int best_j = -1;
		float best_v = -1.0f;
		for (int j = 0; j < static_cast<int>(num_points); j++)
		{
			int real_j = static_cast<int>(jspace.at(j));
			int j_lo = std::max(0, real_j - dx);
			int j_hi = std::min(static_cast<int>(src.shape()[1]), real_j + dx);
			float jreg = 2.0f * (0.5f - std::fabsf(0.5f - static_cast<float>(j)/static_cast<float>(num_points))) + 0.2f;

			float val0 = static_cast<float>(xt::mean(xt::view(src, xt::range(bi_lo, bi_hi), xt::range(j_lo, j_hi)))[0]);
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
	xt::xtensor<float, 2> res({ static_cast<size_t>(num_y), static_cast<size_t>(max_x - min_x)});

	int imax = static_cast<int>(src.shape()[0] - 1);
	xt::xtensor<float, 1> yvals = xt::interp(xt::arange<float>(static_cast<float>(max_x - min_x)) + min_x, curve.x_value, curve.y_value);
	for (int i = 0; i < static_cast<int>(num_y); i++)
		for (int j = 0; j < max_x - min_x; j++)
		{
			int ival = std::max(0, std::min(imax, static_cast<int>(yvals[j] + i - dNeg)));
			res.at(i, j) = src.at(ival, j + min_x);
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
	int crv_neg_size = static_cast<int>(crv_n.size()) - 1;
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
		res.y_value[i] = static_cast<float>(std::get<0>(crv0[i]));
		res.x_value[i] = static_cast<float>(std::get<1>(crv0[i]));
	}
	res.calculateLenParametrization();
	return res;
}

struct RecursiveSegmentInfo
{
	int i0 = -1, i1 = -1, offset = 0;
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

	int midpoint = static_cast<int>(arr2d.shape()[0] / 2);
	auto arr_l = xt::view(arr2d, midpoint, xt::all()) - xt::view(arr2d, 0, xt::all());
	auto arr_r = xt::view(arr2d, -1, xt::all()) - xt::view(arr2d, midpoint, xt::all());

	auto reg_0 = (xt::arange<float>(0.0f, static_cast<float>(arr2d.shape()[1])) - offset) / static_cast<float>(arr2d.shape()[1]);
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

Curve bujo::curves::optimizeCurveBinarySplit(const xt::xtensor<float, 2>& src, const Curve& curve, unsigned max_offset_y, unsigned min_window_x, float reg_coef)
{
	int num_offsets = max_offset_y * 2 + 1;
	xt::xtensor<float, 1> offsets;
	offsets.resize({ static_cast<size_t>(num_offsets) });
	for (int i = 0; i < num_offsets; i++)
		offsets[i] = static_cast<float>(i - max_offset_y);

	xt::xtensor<float, 2> cumulativeIntegral = integral::calcAccumIntegralOverCurve(src, curve, offsets);

	std::vector<std::tuple<float, float>> midpoints;
	midpoints.reserve(cumulativeIntegral.shape()[0] / min_window_x);
	auto recOffsets = buildRecursiveOffsets_(cumulativeIntegral, 0, static_cast<int>(cumulativeIntegral.shape()[0]),
		max_offset_y, min_window_x, reg_coef);
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
	vX[vX.size() - 1] = static_cast<float>(cumulativeIntegral.shape()[0]);
	vY[vY.size() - 1] = std::get<1>(midpoints.back());

	auto denseCurve = getDenseXY(curve);
	const auto& denseX = std::get<0>(denseCurve);
	auto denseOffsets = xt::interp(denseX - xt::amin(denseX)[0], vX, vY);

	Curve res;
	res.x_value = denseX;
	res.y_value = std::get<1>(denseCurve) + denseOffsets;
	res.len_param.resize({ res.x_value.size() });
	res.calculateLenParametrization();
	return res;
}


Curve bujo::curves::optimizeCurve(const xt::xtensor<float, 2>& src, const Curve& curve, unsigned max_offset_y, unsigned min_window_x, float reg_coef)
{
	int num_offsets = max_offset_y * 2 + 1;
	xt::xtensor<float, 1> offsets;
	offsets.resize({ static_cast<size_t>(num_offsets) });
	for (int i = 0; i < num_offsets; i++)
		offsets[i] = static_cast<float>(i - max_offset_y);

	xt::xtensor<float, 2> cumulativeIntegral = integral::calcAccumIntegralOverCurve(src, curve, offsets);
	xt::xtensor<int, 1> curOffsets;
	curOffsets.resize({curve.x_value.size()});
	for (int i = 0; i < curOffsets.size(); i++)
		curOffsets[i] = max_offset_y;

	unsigned window = static_cast<unsigned>(cumulativeIntegral.shape()[0] << 1);
	float xmin = xt::amin(curve.x_value)[0];
	while (window > min_window_x)
	{
		int dneg = static_cast<int>(window / 2);
		int dpos = window - dneg;
		for (int i = 0; i < curOffsets.size(); i++)
		{
			int idx = static_cast<int>(std::floorf(curve.x_value[i] - xmin));
			int idx0 = std::max(0, idx - dneg);
			int idx1 = std::min(static_cast<int>(cumulativeIntegral.shape()[0] - 1), idx + dpos);
			auto xv0 = (xt::view(cumulativeIntegral, idx1, xt::all())- xt::view(cumulativeIntegral, idx0, xt::all())) / static_cast<float>(idx1-idx0);
			auto reg0 = (xt::arange<float>(0.0f, static_cast<float>(xv0.size())) - curOffsets[i]) / static_cast<float>(xv0.size());
			auto reg1 = -reg_coef * xt::pow(xt::abs(reg0), 2.0f);
			curOffsets[i] = bujo::extremum::findLocalMaximaByGradient(xv0 + reg1, curOffsets[i], true);
		}
		window = window >> 1;
	}

	Curve res;
	res.x_value = curve.x_value;
	res.y_value = curve.y_value + xt::cast<float>(curOffsets - max_offset_y);
	res.len_param.resize({ res.x_value.size() });
	res.calculateLenParametrization();
	return res;
}


xt::xtensor<float, 2> calcDistanceFromCurve_(const xt::xtensor<float, 2>& src,
	const xt::xtensor<int, 1> &xPositions, const xt::xtensor<int, 1> &yPositions, unsigned max_offset_y)
{
	xt::xtensor<float, 2> res({xPositions.size(), 2});
	for (unsigned i = 0; i < xPositions.size(); i++)
	{
		int x0 = xPositions[i];
		int y0 = yPositions[i];
		auto v_pos = xt::view(src, xt::range(y0, y0 + static_cast<int>(max_offset_y)), x0);
		auto v_neg = xt::view(src, xt::range(y0, y0 - static_cast<int>(max_offset_y), -1), x0);
		int off_pos = static_cast<int>(xt::argmax(v_pos)[0]);
		int off_neg = static_cast<int>(xt::argmax(v_neg)[0]);
		if (src.at(y0 + off_pos, x0) < 0.5f)
			off_pos = -1;
		if (src.at(y0 - off_neg, x0) < 0.5f)
			off_neg = -1;
		res.at(i, 0) = static_cast<float>(off_neg);
		res.at(i, 1) = static_cast<float>(off_pos);
	}
	return res;
}

xt::xtensor<float, 1> calcQuantileInterpolation_(const xt::xtensor<float, 1>& x, const xt::xtensor<float, 1> &xp, const xt::xtensor<float, 1>& yp, float quantile)
{
	std::vector<float> buffer(xp.size());
	xt::xtensor<float, 1> res;
	res.resize({ x.size() });
	int jprev = 0, jnext = 0;

	for (unsigned i = 0; i < x.size(); i++)
	{
		float split_x = x[i];
		if (i + 1 < x.size())
			split_x = 0.5f * (split_x + x[i + 1]);
		while ((jnext < xp.size()) && (xp[jnext] < split_x))
			jnext++;

		auto yv0 = xt::view(yp, xt::range(jprev, jnext));
		auto yv = xt::filter(yv0, yv0 >= -0.1f);

		if (yv0.size() < 3)
			res.at(i) = -1.0f;
		else
			res.at(i) = bujo::util::calculateQuantile(yv.cbegin(), yv.cend(), quantile, &buffer[0], buffer.size());
		jprev = jnext;
	}
	return res;
}

float calcCurveY2_(const xt::xtensor<float, 1>& x, const xt::xtensor<float, 2>& y)
{
	float res = 0.0f;
	for (int i = 1; i < x.size(); i++)
	{
		float dy = y[i] - y[i - 1];
		res += dy * dy;
	}
	return res;
}
float calcCurveDY2_(const xt::xtensor<float, 1>& x, const xt::xtensor<float, 2>& y)
{
	float res = 0.0f;
	for (int i = 1; i < x.size(); i++)
	{
		if (std::fabsf(x[i] - x[i - 1]) < 1e-7)
			continue;
		float ddy = (y[i] - y[i - 1]) / (x[i] - x[i-1]);
		res += ddy * ddy;
	}
	return res;
}

float calcCurveLoss_(const xt::xtensor<float, 2>& offRange, const xt::xtensor<float, 1>& offActual)
{
	double res = 0.0;
	for (int i = 0; i < offActual.size(); i++)
	{
		if ((offRange.at(i, 0) < 0.0f) && (offRange.at(i, 1) < 0.0f))
			continue;
		float dneg = std::fabsf(offActual.at(i) + offRange.at(i, 0));
		float dpos = std::fabsf(offActual.at(i) - offRange.at(i, 1));
		float dres = 0.0f;
		if ((offRange.at(i, 0) >= 0.0f) && (offRange.at(i, 1) >= 0.0f))
			dres = std::min(dneg, dpos);
		else
		{
			if (offRange.at(i, 0) < 0.0f)
				dres = dpos;
			else
				dres = dneg;
		}
		res += dres;
	}
	return static_cast<float>(res);
}

/*
Curve bujo::curves::optimizeCurveLocal(const xt::xtensor<float, 2>& src, const Curve& curve, int max_offset_y, float quantile, float reg_coef)
{
	//previous attemp, local, hence unstable
	auto denseXY = bujo::curves::getDenseXY(curve);
	const auto& denseX = std::get<0>(denseXY);
	const auto& denseY = std::get<1>(denseXY);
	auto denseDst = calcDistanceFromCurve_(src, xt::cast<int>(denseX), xt::cast<int>(denseY), max_offset_y);
	auto offsetNeg = calcQuantileInterpolation_(curve.x_value, denseX, xt::view(denseDst, xt::all(), 0), quantile);
	auto offsetPos = calcQuantileInterpolation_(curve.x_value, denseX, xt::view(denseDst, xt::all(), 1), quantile);

	std::cout << offsetNeg << "\n";
	std::cout << offsetPos << "\n\n";
	
	return curve;
}*/

float calcIntegralOverLine_(const xt::xtensor<float, 2>& arr2d, xt::xtensor<float, 1> xVals, xt::xtensor<float, 1> yVals)
{
	int min_x = static_cast<int>(std::floorf(xt::amin(xVals)[0]));
	int max_x = static_cast<int>(std::ceilf(xt::amax(xVals)[0]));
	xt::xtensor<float, 1> xs = xt::arange(static_cast<float>(min_x), static_cast<float>(max_x + 1));
	xt::xtensor<float, 1> ys = xt::interp(xs, xVals, yVals);
	float res = 0.0f;
	for (int i = 0; i < xs.size(); i++)
	{
		int i0 = static_cast<int>(std::floor(ys[i]));
		float frac = ys[i] - i0;
		int i1 = i0 + 1;

		i0 = std::max(0, std::min(static_cast<int>(arr2d.shape()[0] - 1), i0));
		i1 = std::max(0, std::min(static_cast<int>(arr2d.shape()[0] - 1), i1));
		if (i + min_x >= arr2d.shape()[1])
			continue;
		float v0 = arr2d.at(i0, i + min_x);
		float v1 = arr2d.at(i1, i + min_x);
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
		yV[chng_id] = static_cast<float>(i);
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
			yV[0] = static_cast<float>(i);
			yV[1] = static_cast<float>(j);
			res.at(i, j) = calcIntegralOverLine_(arr2d, xV, yV);
		}
	return res;
}

xt::xtensor<float, 1> findLineStart_(const xt::xtensor<float, 2>& arr2d, float reg_coef)
{
	int offset = static_cast<int>(arr2d.shape()[0] / 2);
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

Curve bujo::curves::extractCurve(const Curve& src, float p0, float p1)
{
	std::vector<float> ps;
	ps.reserve(src.len_param.size()+2);
	ps.push_back(p0);
	for (int i = 0; i < src.len_param.size(); i++)
	{
		if (src.len_param[i] <= p0)
			continue;
		if (src.len_param[i] > p1)
			break;
		ps.push_back(src.len_param[i]);
	}
	if (ps.back() < p1)
		ps.push_back(p1);

	xt::xtensor<float, 1> pt;
	pt.resize({ ps.size() });
	for (int i = 0; i < pt.size(); i++)
		pt[i] = ps[i];

	Curve res;
	res.x_value = xt::interp(pt, src.len_param, src.x_value);
	res.y_value = xt::interp(pt, src.len_param, src.y_value);
	res.calculateLenParametrization();
	return res;
}

Curve bujo::curves::affineTransformCurve(const Curve& src, float x_offset, float x_scale, float y_offset, float y_scale)
{
	Curve res;
	res.x_value = (src.x_value + x_offset) * x_scale;
	res.y_value = (src.y_value + y_offset) * y_scale;
	res.calculateLenParametrization();
	return res;
}

Curve bujo::curves::shiftCurve(const Curve& src, float yOffset, float clampMin, float clampMax)
{
	Curve res;
	res.x_value = src.x_value;
	res.y_value = xt::clip(src.y_value + yOffset, clampMin, clampMax);
	res.calculateLenParametrization();
	return res;
}

Curve bujo::curves::interpolateCurves(const Curve& src1, const Curve& src2, float alpha)
{
	Curve res;
	xt::xtensor<float, 1> tmp = xt::concatenate(std::make_tuple(src1.x_value, src2.x_value));
	res.x_value = xt::unique(xt::sort(tmp));
	auto y1 = xt::interp(res.x_value, src1.x_value, src1.y_value);
	auto y2 = xt::interp(res.x_value, src2.x_value, src2.y_value);
	res.y_value = y1 * (1 - alpha) + y2 * alpha;
	res.calculateLenParametrization();
	return res;
}

Curve bujo::curves::clipCurve(const xt::xtensor<float, 2>& src, const Curve& curve, unsigned window, unsigned trim)
{
	auto denseXY = bujo::curves::getDenseXY(curve);
	auto &denseX = std::get<0>(denseXY);
	auto &denseY = std::get<1>(denseXY);

	int dneg = window / 2;
	int dpos = window - dneg;

	xt::xtensor<float, 1> r;
	r.resize({ denseX.size() });
	for (int i = 0; i < denseX.size(); i++)
	{
		int j = static_cast<int>(denseX[i]);
		int i0 = std::max(0, static_cast<int>(std::floorf(denseY[i])) - dneg);
		int i1 = std::min(static_cast<int>(src.shape()[0] - 1), static_cast<int>(std::ceilf(denseY[i])) + dpos);
		r[i] = static_cast<float>(xt::mean(xt::view(src, xt::range(i0, i1), j))[0]);
	}

	//find left offset
	int offLeft = 0;
	while (offLeft < r.size() && r[offLeft] <= 1e-3f)
		offLeft++;
	float xMin = denseX[std::min(static_cast<int>(r.size()-1) , std::max(0, offLeft - static_cast<int>(trim)))];
	if (xMin < curve.x_value[0])
		xMin = curve.x_value[0];

	//find right offset
	int offRight = static_cast<int>(r.size()) - 1;
	while (offRight >= 0 && r[offRight] <= 1e-3f)
		offRight--;
	float xMax = denseX[std::min(static_cast<int>(r.size() - 1), std::max(0, offRight + static_cast<int>(trim)))];
	if (xMax > curve.x_value[curve.x_value.size()-1])
		xMax = curve.x_value[curve.x_value.size() - 1];

	std::vector<float> xValues;
	xValues.reserve(curve.x_value.size());
	for (int i = 0; i < curve.x_value.size(); i++)
	{
		if (curve.x_value[i] < xMin)
			continue;
		if (curve.x_value[i] > xMax)
			continue;
		if (xValues.empty())
		{
			if (curve.x_value[i] > xMin + 1e-7)
				xValues.push_back(xMin);
		}
		xValues.push_back(curve.x_value[i]);
	}
	if (xValues.back() < xMax - 1e-7)
		xValues.push_back(xMax);


	Curve res;
	res.x_value.resize({ xValues.size() });
	for (int i = 0; i < xValues.size(); i++)
		res.x_value[i] = xValues[i];
	res.y_value = xt::interp(res.x_value, curve.x_value, curve.y_value);
	res.calculateLenParametrization();
	return res;
}

std::vector<CurveCombination> bujo::curves::generateCurveCombinations(const xt::xtensor<float, 2>& src, const std::vector<Curve>& supportCurves)
{
	//0 step -- check sort order
	std::vector<float> chkOrder(supportCurves.size());
	for (int i = 0; i < supportCurves.size(); i++)
		chkOrder[i] = static_cast<float>(xt::mean(supportCurves[i].y_value)[0]);
	for (int i = 1; i < supportCurves.size(); i++)
		if (chkOrder[i] < chkOrder[i - 1])
			throw std::runtime_error("generateCurveCombinations() expects support curves to be in certain order (increasing mean-y values).");

	std::vector<CurveCombination> res;
	res.reserve(src.shape()[0]);
	//1 step -- offset above, offset=0 not included
	int off0 = static_cast<int>(std::ceilf(xt::amax(supportCurves[0].y_value)[0]));
	for (int i = 0; i < off0; i++)
	{
		CurveCombination cmb;
		cmb.idx1 = cmb.idx2 = 0;
		cmb.alpha = 0.0f;
		cmb.offset = static_cast<float>(-(off0 - i));
		res.push_back(cmb);
	}

	//2 step -- interpolation between, alpha=0 not included
	for (int i = 1; i < supportCurves.size(); i++)
	{
		int i0min = static_cast<int>(std::floorf(xt::amin(supportCurves[i - 1].y_value)[0]));
		int i1max = static_cast<int>(std::ceilf(xt::amax(supportCurves[i].y_value)[0]));
		float aprev = -1.0f;
		for (int j = i0min + 1; j <= i1max; j++)
		{
			float alpha = static_cast<float>(j - i0min) / static_cast<float>(i1max - i0min);
			if (alpha <= aprev)
				continue;
			aprev = alpha;

			CurveCombination cmb;
			cmb.idx1 = i - 1;
			cmb.idx2 = i;
			cmb.alpha = alpha;
			cmb.offset = 0.0f;
			res.push_back(cmb);
		}
	}
	//3 step -- offset below, offset included
	int off1 = static_cast<int>(std::floorf(xt::amin(supportCurves.back().y_value)[0]));
	for (int i = 0; i < off1; i++)
	{
		CurveCombination cmb;
		cmb.idx1 = cmb.idx2 = static_cast<int>(supportCurves.size() )- 1;
		cmb.alpha = 0.0f;
		cmb.offset = static_cast<float>(i);
		res.push_back(cmb);
	}

	return res;
}

float calcCombinedIntegral_(const xt::xtensor<float, 2>& src, const xt::xtensor<int, 1>& xvalues, const Curve& c1, const Curve& c2, float alpha, float offset)
{
	xt::xtensor<float, 1> y0 = xt::interp(xvalues, c1.x_value, c1.y_value);
	if (alpha > 0.0f)
		y0 = y0 * (1 - alpha) + xt::interp(xvalues, c2.x_value, c2.y_value) * alpha;

	y0 = xt::clip(y0 + offset, 0.0f, src.shape()[0] - 1);

	float res = 0.0f;
	for (int j = 0; j < xvalues.size(); j++)
	{
		int i0 = static_cast<int>(std::floorf(y0[j]));
		float frac = y0[j] - i0;
		int i1 = i0 + 1;

		float v = src.at(i0, xvalues[j]);
		if ((frac > 1e-3f) && (i0 + 1 < src.shape()[0]))
			v = v * (1 - frac) + src.at(i0 + 1, xvalues[j]) * frac;
		res += v;
	}
	return res;
}

xt::xtensor<int, 1> generateCombinedXValues_(const Curve& c1, const Curve& c2, float alpha)
{
	float xMin = xt::amin(c1.x_value)[0];
	float xMax = xt::amax(c1.x_value)[0];
	if ((&c1 != &c2) && (alpha > 0.0f))
	{
		xMin = std::min(xMin, xt::amin(c2.x_value)[0]);
		xMax = std::max(xMax, xt::amax(c2.x_value)[0]);
	}
	return xt::arange<int>(static_cast<int>(std::floorf(xMin)), static_cast<int>(std::ceilf(xMax)) + 1);
}

float bujo::curves::calculateCurveCombinationIntegral(const xt::xtensor<float, 2>& src, const std::vector<Curve>& supportCurves, const CurveCombination& combination)
{
	auto xvalues = generateCombinedXValues_(supportCurves.at(combination.idx1), supportCurves.at(combination.idx2), combination.alpha);
	return calcCombinedIntegral_(src, xvalues, supportCurves.at(combination.idx1), supportCurves.at(combination.idx2),
		combination.alpha, combination.offset);
}

xt::xtensor<float, 1> bujo::curves::calculateCurveCombinationIntegral(const xt::xtensor<float, 2>& src, const std::vector<Curve>& supportCurves, const std::vector<CurveCombination>& combination)
{
	int prev_idx1 = -1, prev_idx2 = -1;
	xt::xtensor<float, 1> res, xvalues;
	res.resize({ combination.size() });
	for (int i = 0; i < combination.size(); i++)
	{
		if ((combination[i].idx1 != prev_idx1) || (combination[i].idx2 != prev_idx2))
		{
			xvalues = generateCombinedXValues_(supportCurves.at(combination[i].idx1), supportCurves.at(combination[i].idx2),
				combination[i].alpha);
			prev_idx1 = combination[i].idx1;
			prev_idx2 = combination[i].idx2;
		}
		res[i] = calcCombinedIntegral_(src, xvalues, supportCurves.at(combination[i].idx1), supportCurves.at(combination[i].idx2),
			combination[i].alpha, combination[i].offset);
	}
	return res;
}

std::vector<unsigned> bujo::curves::selectCurveCandidates(const xt::xtensor<float, 1>& vals, unsigned window, float min_value, float max_ratio)
{
	std::vector<unsigned> res;

	//1 step locate local maximas
	auto s1 = bujo::extremum::getLocalMaximas(vals);

	//2 step filter by maxima in window and ratio value
	auto wmax = bujo::filters::filterMax1D(vals, window);
	auto wmin1 = bujo::filters::filterMin1D1Way(vals, static_cast<int>(window));
	auto wmin2 = bujo::filters::filterMin1D1Way(vals, -static_cast<int>(window));
	auto wmin = xt::maximum(wmin1, wmin2);

	std::vector<unsigned> s2;
	s2.reserve(s1.size());
	for (int i = 0; i < s1.size(); i++)
	{
		if (vals[s1[i]] < min_value)
			continue;
		if (vals[s1[i]] < wmax[s1[i]])
			continue;
		if (wmin[s1[i]] / vals[s1[i]] > max_ratio)
			continue;
		s2.push_back(s1[i]);
	}

	//3 step filter "flat" regions, leaving only most center ones
	auto s3 = bujo::extremum::filterAdjacentExtremas(s2);

	return s3;
}

Curve bujo::curves::generateCurve(const std::vector<Curve>& supportCurves, const CurveCombination& curveCombination, float clipMin, float clipMax)
{
	if ((curveCombination.idx1 != curveCombination.idx2) && (curveCombination.alpha > 0.0f))
		return shiftCurve(interpolateCurves(supportCurves.at(curveCombination.idx1),
			supportCurves.at(curveCombination.idx2), curveCombination.alpha), curveCombination.offset, clipMin, clipMax);
	return shiftCurve(supportCurves.at(curveCombination.idx1), curveCombination.offset, clipMin, clipMax);
}

inline unsigned getClosestLocalMin1D_(const xt::xtensor<float, 1>& arr)
{
	/*
	off_less = np.argmax(arr1d[1:]<arr1d[:-1])
	msk_min = arr1d[1:]>=arr1d[:-1]
	off = np.argmax(msk_min[off_less:])+off_less
	return off
	*/
	xt::xtensor<int, 1> arrL = xt::view(arr, xt::range(1, xt::placeholders::_)) < xt::view(arr, xt::range(xt::placeholders::_, -1));
	xt::xtensor<int, 1> arrGE = xt::view(arr, xt::range(1, xt::placeholders::_)) >= xt::view(arr, xt::range(xt::placeholders::_, -1));
	auto off0 = xt::argmax(arrL)[0];
	return static_cast<unsigned>(off0 + xt::argmax(xt::view(arrGE, xt::range(off0, xt::placeholders::_)))[0]);
}

std::tuple<unsigned, unsigned> bujo::curves::getCurveHeight(const xt::xtensor<float, 2>& src, const Curve& curve, unsigned max_offset, float reg_coef)
{
	/*
	offsets = np.arange(-max_offset, max_offset+1)
	values = np.array([np.sum(src[np.maximum(0, np.minimum(src.shape[0]-1, crv_i+off)), crv_j]) for off in offsets])
	off_r = get_closest_local_min(values[max_offset:]+additive_regularization)
	off_l = get_closest_local_min(values[:(max_offset+1)][::-1]+additive_regularization)
	return (-off_l, off_r)
	*/
	xt::xtensor<float, 1> offsets = xt::arange<float>(-static_cast<float>(max_offset), static_cast<float>(max_offset + 1));
	auto integrals = bujo::curves::integral::calcIntegralOverCurve(src, curve, offsets);
	xt::xtensor<float, 1> vreg = reg_coef * xt::pow(xt::linspace(0.0f, 2.0f, max_offset + 1), 2.0f);

	auto vneg = xt::view(integrals, xt::range(max_offset, xt::placeholders::_, -1)) + vreg;
	auto vpos = xt::view(integrals, xt::range(max_offset, xt::placeholders::_)) + vreg;

	return std::make_tuple(getClosestLocalMin1D_(vneg), getClosestLocalMin1D_(vpos));
}

std::vector<std::tuple<float, float>> bujo::curves::locateWordsInLine(const xt::xtensor<float, 2>& srcLine, unsigned window, float min_value)
{
	auto v0 = xt::amax(srcLine, 0);
	auto vw = bujo::filters::filterMax1D(v0, window);
	auto lms = bujo::extremum::getLocalMinimas(vw);

	std::vector<std::tuple<float, float>> res;
	res.reserve(lms.size());

	for (int i = 1; i < lms.size(); i++)
	{
		if (lms[i] == lms[i - 1] + 1)
			continue;
		int j0 = lms[i - 1];
		int j1 = lms[i];
		float val = static_cast<float>(xt::mean(xt::view(v0, xt::range(j0, j1)))[0]);
		if (val < min_value)
			continue;
		res.emplace_back(static_cast<float>(j0), static_cast<float>(j1));
	}

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
	auto xyLocs = getDenseXY(curve);
	const auto& xLocs = std::get<0>(xyLocs);
	const auto& yLocs = std::get<1>(xyLocs);
	int clampMax = static_cast<int>(src.shape()[0]) - 1;
	for (int i = 0; i < offsets.size(); i++)
	{
		float val = 0.0f;
		for (int j = 0; j < xLocs.size(); j++)
		{
			int real_j = static_cast<int>(std::roundf(xLocs[j]));
			int real_i = static_cast<int>(std::roundf(yLocs[j] + offsets[i]));
			real_i = std::max(0, std::min(clampMax, real_i));
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
	auto xyLocs = getDenseXY(curve);
	const auto& xLocs = std::get<0>(xyLocs);
	const auto& yLocs = std::get<1>(xyLocs);
	int clampMax = static_cast<int>(src.shape()[0]) - 1;

	xt::xtensor<float, 2> res({ xLocs.size(), offsets.size() });
	
	for (int i = 0; i < offsets.size(); i++)
	{
		float val = 0.0f;
		for (int j = 0; j < xLocs.size(); j++)
		{
			int real_j = static_cast<int>(std::roundf(xLocs[j]));
			int real_i = static_cast<int>(std::roundf(yLocs[j] + offsets[i]));
			real_i = std::max(0, std::min(clampMax, real_i));
			val += src.at(real_i, real_j);
			res.at(j, i) = val;
		}
	}
	return std::move(res);
}


std::tuple<xt::xtensor<float, 1>, xt::xtensor<float, 1>> bujo::curves::getDenseXY(const Curve& curve)
{
	float xMin = xt::amin(curve.x_value)[0], xMax = xt::amax(curve.x_value)[0];
	int jMin = static_cast<int>(std::floorf(xMin));
	int jMax = static_cast<int>(std::ceilf(xMax));
	int numPoints = jMax - jMin + 1;

	xt::xtensor<float, 1> resX;
	resX.resize({ static_cast<size_t>(numPoints) });
	for (int j = 0; j < numPoints; j++)
		resX[j] = static_cast<float>(jMin + j);

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

