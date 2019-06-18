#pragma once
#include <xtensor/xtensor.hpp>
#include <vector>

namespace bujo
{
	namespace extremum
	{
		std::tuple<unsigned, unsigned> getNonTrivialRange(const xt::xtensor<float, 1>& src);
		std::tuple<unsigned, unsigned> getPositiveSuperRange(const xt::xtensor<float, 1>& src);
		std::vector<std::tuple<unsigned, unsigned>> getPositiveRanges(const xt::xtensor<float, 1>& src);

		std::tuple<unsigned, unsigned> getSuperRangeQuantile(const xt::xtensor<float, 1>& src, float quantile);


		std::vector<unsigned> getLocalMinimas(const xt::xtensor<float, 1>& src);
		std::vector<unsigned> getLocalMaximas(const xt::xtensor<float, 1>& src);

		std::vector<unsigned> filterAdjacentExtremas(const std::vector<unsigned>& ids);
		std::vector<unsigned> filterCloseMinimas(const xt::xtensor<float, 1>& src,
			const std::vector<unsigned>& ids, unsigned window);
		std::vector<unsigned> filterCloseMaximas(const xt::xtensor<float, 1>& src,
			const std::vector<unsigned>& ids, unsigned window);

		unsigned findLocalMaximaByGradient(const xt::xtensor<float, 1>& src, unsigned i0, bool strictExtremum);
		unsigned findLocalMinimaByGradient(const xt::xtensor<float, 1>& src, unsigned i0, bool strictExtremum);
	}
}
