#include "filters.h"
#include <xtensor/xview.hpp>
#include <exception>
#include <vector>
#include "util/quantiles.h"

xt::xtensor<float, 2> bujo::filters::filterVariance2DV(const xt::xtensor<float, 2>& src, unsigned size)
{
	auto x1 = xt::view(src, xt::range(1, xt::placeholders::_), xt::all()) -
		xt::view(src, xt::range(xt::placeholders::_, -1), xt::all());
	xt::xtensor<float, 2> x2 = xt::abs(x1);
	auto x3 = xt::cumsum(x2, 0);
	auto x4 = xt::view(x3, xt::range(size, xt::placeholders::_), xt::all()) -
		xt::view(x3, xt::range(xt::placeholders::_, -int(size)), xt::all());
	return x4 / float(size);
}

xt::xtensor<float, 2> bujo::filters::filterVariance2DH(const xt::xtensor<float, 2>& src, unsigned size)
{
	auto x1 = xt::view(src, xt::all(), xt::range(1, xt::placeholders::_)) -
		xt::view(src, xt::all(), xt::range(xt::placeholders::_, -1));
	xt::xtensor<float, 2> x2 = xt::abs(x1);
	auto x3 = xt::cumsum(x2, 1);
	auto x4 = xt::view(x3, xt::all(), xt::range(int(size), xt::placeholders::_)) -
		xt::view(x3, xt::all(), xt::range(xt::placeholders::_, -int(size)));
	return x4 / float(size);
}

xt::xtensor<float, 2> bujo::filters::filterQuantile2D(const xt::xtensor<float, 2>& src, unsigned size_w, unsigned size_h, float quantile)
{
	std::vector<float> vbuffer(size_w * size_h);
	int res_h = static_cast<int>(src.shape()[0]) - static_cast<int>(size_h) + 1;
	int res_w = static_cast<int>(src.shape()[1]) - static_cast<int>(size_w) + 1;
	if ((res_w <= 0) || (res_h <= 0))
		throw std::runtime_error("Function filterQuantile2D() received kernel size that is greater than image!");

	xt::xtensor<float, 2> res = xt::zeros<float>({ res_h, res_w });
	for(unsigned i = 0; i < res.shape()[0]; i++)
		for (unsigned j = 0; j < res.shape()[1]; j++)
		{
			auto x = xt::view(src, xt::range(i, i + size_h), xt::range(j, j + size_w));
			float f = bujo::util::calculateQuantile(x.cbegin(), x.cend(), quantile, &vbuffer[0], vbuffer.size());
			res.at(i, j) = f;
		}
	return res;
}

xt::xtensor<float, 2> bujo::filters::filterVarianceQuantileV(const xt::xtensor<float, 2>& src, unsigned size_w, unsigned size_h, float quantile)
{
	return filterQuantile2D(filterVariance2DV(src, size_h), size_w, 1, quantile);
}

xt::xtensor<float, 2> bujo::filters::filterVarianceQuantileH(const xt::xtensor<float, 2>& src, unsigned size_w, unsigned size_h, float quantile)
{
	return filterQuantile2D(filterVariance2DH(src, size_w), 1, size_h, quantile);
}

xt::xtensor<float, 2> bujo::filters::filterVarianceQuantileVH(const xt::xtensor<float, 2>& src, unsigned size_w, unsigned size_h, float quantile_w, float quantile_h)
{
	auto xh = filterVarianceQuantileH(src, size_w, size_h, quantile_w);
	auto xv = filterVarianceQuantileV(src, size_w, size_h, quantile_h);
	return xt::view(xv, xt::all(), xt::range(1, -1)) * xt::view(xh, xt::range(1, -1), xt::all());
}

xt::xtensor<float, 2> kernel_gaussian_(float sigma, float cutoff)
{
	double sigma2 = sigma * sigma;
	
	size_t ksize = 1;
	while (1)
	{
		double dst2 = static_cast<double>(ksize * ksize); //check only (0, ksize) point
		double wgt = std::exp(-dst2 / (2.0 * sigma2)) / (2.0 * 3.1415 * sigma2); //actual normalization coef is not relevant
		if (wgt < cutoff)
			break;

		ksize++;
	}
	if (!(ksize & 1))
		ksize++;

	int c = static_cast<int>(ksize >> 1);
	xt::xtensor<double, 2> res({ ksize, ksize });
	for(unsigned i = 0; i < ksize; i++)
		for (unsigned j = 0; j < ksize; j++)
		{
			int dx = static_cast<int>(i) - c;
			int dy = static_cast<int>(j) - c;
			double dst2 = dx * dx + dy * dy;
			res.at(i, j) = std::exp(-dst2 / (2.0 * sigma2)); //do not normalize
		}
	return xt::cast<float>(res / xt::sum(res)[0]);
}

//very suboptimal implementation -- can implement as two blurs -- horizontal and vertical
xt::xtensor<float, 2> bujo::filters::filterGaussian2D(const xt::xtensor<float, 2>& src, float sigma)
{
	xt::xtensor<float, 2> res({src.shape()[0], src.shape()[1]});
	auto kernel = kernel_gaussian_(sigma, 1e-3f);
	int ksize = static_cast<int>(kernel.shape()[0]);
	int center = ksize >> 1;


	for(int i = 0; i < res.shape()[0]; i++)
		for (int j = 0; j < res.shape()[1]; j++)
		{
			float tres = 0.0;
			for(int di = -center; di <= center; di++)
				for (int dj = -center; dj <= center; dj++)
				{
					int si = i + di, sj = j + dj;
					if ((si < 0) || (si >= src.shape()[0]))
						continue;
					if ((sj < 0) || (sj >= src.shape()[1]))
						continue;
					tres += kernel.at(di + center, dj + center) * src.at(si, sj);
				}
			res.at(i, j) = tres;
		}
	return res;
}

/*
Calculates local maxima filter and applies cutoff

Keyword arguments:
	i -- row id
	d1 -- width of local-maxima
	d2 -- test-range for local-maxima
	cutoff -- minimal value for local_maxima

Returns ndarray of shape src.shape
*/
xt::xtensor<float, 2> bujo::filters::filterLocalMax2DV(const xt::xtensor<float, 2>& src,
	unsigned window_size, unsigned slack_size, float threshold)
{
	/*
def local_maxima_v(src, d1, d2, cutoff):
	def row_local_maxima_v(i):
		ilo1 = max(0, i-d1)
		ilo2 = max(0, i-d2)
		ihi1 = min(src.shape[0], i+d1+1)
		ihi2 = min(src.shape[0], i+d2+1)
		m1 = np.max(src[ilo1:ihi1], axis=0)
		m2 = np.max(src[ilo2:ihi2], axis=0)
		return (m1 >= m2)*(m2 > cutoff)
	return np.array([row_local_maxima_v(i) for i in range(src.shape[0])])
	*/
	int dwin_neg = window_size / 2;
	int dwin_pos = window_size - dwin_neg;
	int dslack_neg = slack_size / 2;
	int dslack_pos = slack_size - dslack_neg;

	xt::xtensor<float, 2> res({ src.shape()[0], src.shape()[1] });
	for (int i = 0; i < src.shape()[0]; i++)
	{
		int window_lo = std::max(0, i - dwin_neg);
		int window_hi = std::min(static_cast<int>(src.shape()[0]), i + dwin_pos);
		int slack_lo = std::max(0, i - dslack_neg);
		int slack_hi = std::min(static_cast<int>(src.shape()[0]), i + dslack_pos);
		auto m_window = xt::amax(xt::view(src, xt::range(window_lo, window_hi), xt::all()), 0);
		auto m_slack = xt::amax(xt::view(src, xt::range(slack_lo, slack_hi), xt::all()), 0);
		xt::view(res, i, xt::all()) = xt::cast<float>((m_slack >= m_window) * (m_window >= threshold));
	}
	return res;
}

xt::xtensor<float, 1> bujo::filters::filterMax1D(const xt::xtensor<float, 1>& src, unsigned window)
{
	xt::xtensor<float, 1> res;
	res.resize({ src.size() });
	int dneg = window / 2;
	int dpos = window - dneg;
	for (int i = 0; i < src.size(); i++)
	{
		int i0 = std::max(0, i - dneg);
		int i1 = std::min(static_cast<int>(src.size()) - 1, i + dpos);
		res[i] = xt::amax(xt::view(src, xt::range(i0, i1)))[0];
	}
	return res;
}

xt::xtensor<float, 1> bujo::filters::filterMin1D(const xt::xtensor<float, 1>& src, unsigned window)
{
	xt::xtensor<float, 1> res;
	res.resize({ src.size() });
	int dneg = window / 2;
	int dpos = window - dneg;
	for (int i = 0; i < src.size(); i++)
	{
		int i0 = std::max(0, i - dneg);
		int i1 = std::min(static_cast<int>(src.size()) - 1, i + dpos);
		res[i] = xt::amin(xt::view(src, xt::range(i0, i1)))[0];
	}
	return res;
}

xt::xtensor<float, 1> bujo::filters::filterMax1D1Way(const xt::xtensor<float, 1>& src, int window)
{
	xt::xtensor<float, 1> res;
	res.resize({ src.size() });
	int dneg = window / 2;
	int dpos = window - dneg;
	for (int i = 0; i < src.size(); i++)
	{
		if (window >= 0)
			res[i] = xt::amax(xt::view(src, xt::range(i, i + window)))[0];
		else
			res[i] = xt::amax(xt::view(src, xt::range(i, i + window, -1)))[0];
	}
	return res;
}

xt::xtensor<float, 1> bujo::filters::filterMin1D1Way(const xt::xtensor<float, 1>& src, int window)
{
	xt::xtensor<float, 1> res;
	res.resize({ src.size() });
	int dneg = window / 2;
	int dpos = window - dneg;
	int imax = static_cast<int>(src.size()) - 1;
	for (int i = 0; i < src.size(); i++)
	{
		if (window >= 0)
			res[i] = xt::amin(xt::view(src, xt::range(i, std::min(imax, i + window))))[0];
		else
			res[i] = xt::amin(xt::view(src, xt::range(i, std::max(0, i + window), -1)))[0];
	}
	return res;
}
