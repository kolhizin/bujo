#include "filters.h"
#include <xtensor/xview.hpp>
#include <exception>
#include <vector>
#include "util/quantiles.h"

xt::xarray<float> bujo::filters::filterVariance2DV(const xt::xarray<float>& src, unsigned size)
{
	/* python implementation:
	var = np.cumsum(np.abs(src[1:,:]-src[:-1,:]), axis=0)
	mean = (var[sz[0]:,:]-var[:-sz[0],:]) / sz[0]
	*/
	auto x1 = xt::view(src, xt::range(1, xt::placeholders::_), xt::all()) -
		xt::view(src, xt::range(xt::placeholders::_, -1), xt::all());
	auto x2 = xt::cumsum(xt::abs(x1), 0);
	auto x3 = xt::view(x2, xt::range(size, xt::placeholders::_), xt::all()) -
		xt::view(x2, xt::range(xt::placeholders::_, -int(size)), xt::all());
	return x3 / float(size);
}

xt::xarray<float> bujo::filters::filterVariance2DH(const xt::xarray<float>& src, unsigned size)
{
	/* python implementation:
	var = np.cumsum(np.abs(src[:,1:]-src[:,:-1]), axis=1)
	mean = (var[:,sz[1]:]-var[:,:-sz[1]]) / sz[1]
	*/
	auto x1 = xt::view(src, xt::all(), xt::range(1, xt::placeholders::_)) -
		xt::view(src, xt::all(), xt::range(xt::placeholders::_, -1));
	auto x2 = xt::cumsum(xt::abs(x1), 1);
	auto x3 = xt::view(x2, xt::all(), xt::range(size, xt::placeholders::_)) -
		xt::view(x2, xt::all(), xt::range(xt::placeholders::_, -int(size)));
	return x3 / float(size);
}

xt::xarray<float> bujo::filters::filterQuantile2D(const xt::xarray<float>& src, unsigned size_w, unsigned size_h, float quantile)
{
	std::vector<float> vbuffer(size_w * size_h);
	int res_h = static_cast<int>(src.shape()[0]) - static_cast<int>(size_h) + 1;
	int res_w = static_cast<int>(src.shape()[1]) - static_cast<int>(size_w) + 1;
	if ((res_w <= 0) || (res_h <= 0))
		throw std::runtime_error("Function filterQuantile2D() received kernel size that is greater than image!");

	xt::xarray<float> res = xt::zeros<float>({ res_h, res_w });
	for(unsigned i = 0; i < res.shape()[0]; i++)
		for (unsigned j = 0; j < res.shape()[1]; j++)
		{
			auto x = xt::view(src, xt::range(i, i + size_w), xt::range(j, j + size_h));
			float f = bujo::util::calculateQuantile(x.cbegin(), x.cend(), quantile, &vbuffer[0], vbuffer.size());
			res.at(i, j) = f;
		}
	return res;
}

xt::xarray<float> bujo::filters::filterVarianceQuantileV(const xt::xarray<float>& src, unsigned size_w, unsigned size_h, float quantile)
{
	return filterQuantile2D(filterVariance2DV(src, size_h), size_w, 1, quantile);
}

xt::xarray<float> bujo::filters::filterVarianceQuantileH(const xt::xarray<float>& src, unsigned size_w, unsigned size_h, float quantile)
{
	return filterQuantile2D(filterVariance2DH(src, size_w), 1, size_h, quantile);
}

xt::xarray<float> bujo::filters::filterVarianceQuantileVH(const xt::xarray<float>& src, unsigned size_w, unsigned size_h, float quantile_v, float quantile_h)
{
	auto xh = filterVarianceQuantileH(src, size_w, size_h, quantile_h);
	auto xv = filterVarianceQuantileV(src, size_w, size_h, quantile_v);
	return xt::view(xv, xt::all(), xt::range(1, -1)) * xt::view(xh, xt::range(1, -1), xt::all());
}
