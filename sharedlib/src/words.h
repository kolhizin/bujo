#pragma once
#include <xtensor/xtensor.hpp>
#include <vector>
#include "curves.h"

namespace bujo
{
	namespace words
	{
		struct Word
		{
			float param0, param1;
		};

		/*
		Locates word in src, which should be image of single line (extracted by methods from bujo::curves).
		*/
		std::vector<Word> locateWordsInLine(const xt::xtensor<float, 2>& src, unsigned window, float min_value);
		xt::xtensor<float, 2> extractWordFromLine(const xt::xtensor<float, 2>& src, const Word& word);
		xt::xtensor<float, 2> extractWord(const xt::xtensor<float, 2>& src, const bujo::curves::Curve &curve, const Word& word, unsigned neg_height, unsigned pos_height);
	}
}