#include "curves.h"
#include "extremum.h"
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

std::vector<std::tuple<float, float>> makeCurve1_(const xt::xtensor<float, 2>& src,)

Curve bujo::curves::generateCurve(const xt::xtensor<float, 2>& src, CurveGenerationOptions options)
{
	return Curve();
}


/*

def make_curve_1way(i0, j0, step, src, grid, angles, kernel_size, reg_coef, reg_power):
	"""Calculates curve from starting position (i0, j0) with directed step. Each step is in direction of regularized local angle.

	Keyword arguments:
	i0, j0 -- starting position
	step -- maximal step
	src -- ndarray of source values
	grid -- global grid
	angles -- array with angles in radians
	kernel_size -- size of region to determine local angle
	reg_coef -- mean value in subregion will be normalized on this value, i.e. np.mean(subgrid)/reg_coef
	reg_power -- np.power(np.mean(subgrid)/reg_coef, reg_power) -- full regularization term

	Returns list of (i, j)-coordinates of curve
	"""
	isz = kernel_size[0]//2
	jsz = kernel_size[1]//2
	def calc_angle(i, j): #function to calculate local angle
		r_i0 = max(0, i-isz)
		r_i1 = min(src.shape[0], i+isz+1)
		r_j0 = max(0, j-jsz)
		r_j1 = min(src.shape[1], j+jsz+1)
		return local_radon.find_local_angle(src, grid, angles, r_i0, r_i1, r_j0, r_j1, reg_coef, reg_power)

	res = [(i0, j0)]
	while True:
		ci, cj = res[-1]
		# step like Runge-Kutta, i.e. we take 0.5 step in local angle, calculate new angle and take full step in that direction
		angle0 = calc_angle(ci, cj)
		di = step * np.sin(-angle0) * 0.5
		dj = step * np.cos(-angle0) * 0.5
		angle = calc_angle(int(np.round(ci+di)), int(np.round(cj+dj)))
		di = step * np.sin(-angle)
		dj = step * np.cos(-angle)
		ni = int(np.round(ci+di))
		nj = int(np.round(cj+dj))

		if ni < 0 or ni >= src.shape[0] or nj < 0 or nj >= src.shape[1]:
			# renormalize when we hit border
			ri = max(0, min(src.shape[0]-1, ni))
			rj = max(0, min(src.shape[1]-1, nj))

			rdi = 1.0
			rdj = 1.0
			if np.abs(di) > 1e-5:
				rdi = (ri - ci) / di
			if np.abs(dj) > 1e-5:
				rdj = (rj - cj) / dj

			rstep = min(rdi, rdj) * 0.9

			ni = int(np.round(ci+di*rstep))
			nj = int(np.round(cj+dj*rstep))
			res.append((ni, nj))
			break

		res.append((ni, nj))
	return res

*/
