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
