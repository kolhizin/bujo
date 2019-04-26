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

		std::vector<unsigned> getLocalMinimas(const xt::xtensor<float, 1>& src);
		std::vector<unsigned> getLocalMaximas(const xt::xtensor<float, 1>& src);
	}
}
