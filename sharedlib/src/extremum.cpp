#include "extremum.h"
#include "util/quantiles.h"

using namespace bujo::extremum;

std::tuple<unsigned, unsigned> bujo::extremum::getNonTrivialRange(const xt::xtensor<float, 1>& src)
{
	xt::xtensor<float, 1> x = xt::abs(src);
	unsigned first = 0, last = 0;
	bool found = false;
	for(unsigned i = 0; i < x.size(); i++)
		if (x[i] > 0.0f)
		{
			if (!found)
			{
				first = i;
				found = true;
			}
			last = i;
		}
	if (!found)
		return std::tuple<unsigned, unsigned>(x.size(), 0);
	return std::tuple<unsigned, unsigned>(first, last+1);
}

std::tuple<unsigned, unsigned> bujo::extremum::getPositiveSuperRange(const xt::xtensor<float, 1>& src)
{
	unsigned first = 0, last = 0;
	bool found = false;
	for (unsigned i = 0; i < src.size(); i++)
		if (src[i] > 0.0f)
		{
			if (!found)
			{
				first = i;
				found = true;
			}
			last = i;
		}
	if (!found)
		return std::tuple<unsigned, unsigned>(src.size(), 0);
	return std::tuple<unsigned, unsigned>(first, last + 1);
}

std::vector<std::tuple<unsigned, unsigned>> bujo::extremum::getPositiveRanges(const xt::xtensor<float, 1>& src)
{
	bool positive = false;
	unsigned prev = 0;
	std::vector<std::tuple<unsigned, unsigned>> res;
	res.reserve(20);

	for(int i = 0; i < src.size(); i++)
		if (src[i] > 0.0f)
		{
			if (!positive)
			{
				prev = i;
				positive = true;
			}
		}
		else
		{
			if (positive)
			{
				positive = false;
				res.push_back(std::make_tuple(prev, i + 1));
			}
		}
	return res;
}

std::tuple<unsigned, unsigned> bujo::extremum::getSuperRangeQuantile(const xt::xtensor<float, 1>& src, float quantile)
{
	std::vector<float> buff(src.size());
	float qres = bujo::util::calculateQuantile(src.cbegin(), src.cend(), quantile, &buff[0], buff.size());
	return getPositiveSuperRange(src - qres);
}

std::vector<unsigned> bujo::extremum::getLocalMinimas(const xt::xtensor<float, 1>& src)
{
	std::vector<unsigned> ids;
	ids.reserve(src.size() / 20);

	int lm_start = 0;
	bool descending = true;
	for (int i = 1; i < src.size(); i++)
	{
		if (src.at(i) > src.at(i - 1))
		{
			if (descending)
			{
				for(int j = lm_start; j <= i-1; j++)
					ids.push_back(j);
			}
			descending = false;
		}
		else if (src.at(i) < src.at(i - 1))
		{
			lm_start = i;
			descending = true;
		}
	}

	if (descending)
		for (int j = lm_start; j < src.size(); j++)
			ids.push_back(j);

	return ids;
}

std::vector<unsigned> bujo::extremum::getLocalMaximas(const xt::xtensor<float, 1>& src)
{
	std::vector<unsigned> ids;
	ids.reserve(src.size() / 20);

	int lm_start = 0;
	bool ascending = true;
	for (int i = 1; i < src.size(); i++)
	{
		if (src.at(i) < src.at(i - 1))
		{
			if (ascending)
			{
				for (int j = lm_start; j <= i - 1; j++)
					ids.push_back(j);
			}
			ascending = false;
		}
		else if (src.at(i) > src.at(i - 1))
		{
			lm_start = i;
			ascending = true;
		}
	}

	if (ascending)
		for (int j = lm_start; j < src.size(); j++)
			ids.push_back(j);

	return ids;
}

unsigned bujo::extremum::findLocalMaximaByGradient(const xt::xtensor<float, 1>& src, unsigned i0, bool strictExtremum)
{
	int i = std::max(0, std::min(static_cast<int>(src.size()) - 1, static_cast<int>(i0)));
	int iprev = i;
	while (1)
	{
		float dpos = (i >= src.size() - 1 ? -1.0f : src[i + 1] - src[i]);
		float dneg = (i <= 0 ? -1.0f : src[i - 1] - src[i]);
		if ((dpos < 0.0f) && (dneg < 0.0f))
			return static_cast<unsigned>(i);
		if (dpos > dneg)
		{
			iprev = i;
			i++;
			continue;
		}
		if (dpos < dneg)
		{
			iprev = i;
			i--;
			continue;
		}
		if (!strictExtremum)
			return static_cast<unsigned>(i);
		int di = i - iprev;
		iprev = i;
		if (di == 0)
			i += 1;
		else
			i += di;
	}
	return 0;
}

unsigned bujo::extremum::findLocalMinimaByGradient(const xt::xtensor<float, 1>& src, unsigned i0, bool strictExtremum)
{
	int i = std::max(0, std::min(static_cast<int>(src.size()) - 1, static_cast<int>(i0)));
	int iprev = i;
	while (1)
	{
		float dpos = (i >= src.size() - 1 ? 1.0f : src[i + 1] - src[i]);
		float dneg = (i <= 0 ? 1.0f : src[i - 1] - src[i]);
		if ((dpos > 0.0f) && (dneg > 0.0f))
			return static_cast<unsigned>(i);
		if (dpos < dneg)
		{
			iprev = i;
			i++;
			continue;
		}
		if (dpos > dneg)
		{
			iprev = i;
			i--;
			continue;
		}
		if (!strictExtremum)
			return static_cast<unsigned>(i);
		int di = i - iprev;
		iprev = i;
		if (di == 0)
			i += 1;
		else
			i += di;
	}
	return 0;
}
