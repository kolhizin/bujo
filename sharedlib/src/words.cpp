#include "words.h"
#include "filters.h"
#include "extremum.h"
#include <xtensor/xview.hpp>

using namespace bujo::words;

std::vector<Word> bujo::words::locateWordsInLine(const xt::xtensor<float, 2>& src, unsigned window, float min_value)
{
	auto v0 = xt::amax(src, 0);
	auto vw = bujo::filters::filterMax1D(v0, window);
	auto lms = bujo::extremum::getLocalMinimas(vw);
	int x1 = static_cast<int>(src.shape()[1] - 1);

	std::vector<Word> res;
	res.reserve(lms.size());

	for (int i = 1; i < lms.size(); i++)
	{
		if (lms[i] == lms[i - 1] + 1)
			continue;
		int j0 = lms[i - 1];
		int j1 = lms[i];
		float val = xt::mean(xt::view(v0, xt::range(j0, j1)))[0];
		if (val < min_value)
			continue;
		Word w;
		w.param0 = static_cast<float>(j0) / x1;
		w.param1 = static_cast<float>(j1) / x1;
		res.push_back(w);
	}

	return res;
}

xt::xtensor<float, 2> bujo::words::extractWordFromLine(const xt::xtensor<float, 2>& src, const Word& word)
{
	int j0 = static_cast<int>(std::floorf(word.param0 * (src.shape()[1] - 1)));
	int j1 = static_cast<int>(std::ceilf(word.param1 * (src.shape()[1] - 1)));
	std::cout << j0 << " " << j1 << "\n";
	return xt::view(src, xt::all(), xt::range(j0, j1));
}

xt::xtensor<float, 2> bujo::words::extractWord(const xt::xtensor<float, 2>& src, const bujo::curves::Curve& curve, const Word& word, unsigned neg_height, unsigned pos_height)
{
	bujo::curves::Curve crv = bujo::curves::extractCurve(curve, word.param0, word.param1);
	return bujo::curves::extractCurveRegion(src, crv, neg_height, pos_height);
}
