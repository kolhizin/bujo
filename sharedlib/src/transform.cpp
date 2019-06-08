#include "transform.h"
#include "util/quantiles.h"
#include "radon.h"
#include "filters.h"
#include <xtensor/xview.hpp>
#include <xtensor/xsort.hpp>

#include <src/util/cv_ops.h>

using namespace bujo::transform;
constexpr float pi_f = 3.1415926f;


xt::xtensor<float, 2> impl_resize_image_bilinear_(const xt::xtensor<float, 2>& src, float factor)
{
	size_t new_h = static_cast<size_t>(std::ceilf(src.shape()[0] * factor));
	size_t new_w = static_cast<size_t>(std::ceilf(src.shape()[1] * factor));
	xt::xtensor<float, 2> res({ new_h, new_w });
	for (unsigned i = 0; i < new_h; i++)
	{
		int i0 = static_cast<int>(std::floorf(i / factor));
		int i1 = std::min(static_cast<int>(src.shape()[0])-1, std::max(0, static_cast<int>(std::floorf((i+1) / factor))-1));
		for (unsigned j = 0; j < new_w; j++)
		{
			int j0 = static_cast<int>(std::floorf(j / factor));
			int j1 = std::min(static_cast<int>(src.shape()[1]) - 1, std::max(0, static_cast<int>(std::floorf((j + 1) / factor))-1));
			res.at(i, j) = 0.25f * (src.at(i0, j0) + src.at(i0, j1) + src.at(i1, j0) + src.at(i1, j1));
		}
	}
	return res;
}

xt::xtensor<float, 2> bujo::transform::resizeImage(const xt::xtensor<float, 2>& src, float factor)
{
	xt::xtensor<float, 2> res = src;
	while (factor < 0.5f)
	{
		res = impl_resize_image_bilinear_(res, 0.5f);
		factor *= 2.0f;
	}
	return impl_resize_image_bilinear_(res, factor);
}

xt::xtensor<float, 2> bujo::transform::rotateImage(const xt::xtensor<float, 2>& src, float angle)
{
	float a_sin = std::sinf(angle), a_cos = std::cosf(angle);
	float new_fh = src.shape()[0] * std::fabsf(a_cos) + src.shape()[1] * std::fabsf(a_sin);
	float new_fw = src.shape()[0] * std::fabsf(a_sin) + src.shape()[1] * std::fabsf(a_cos);
	size_t new_h = static_cast<size_t>(std::ceilf(new_fh));
	size_t new_w = static_cast<size_t>(std::ceilf(new_fw));
	xt::xtensor<float, 2> res({new_h, new_w});

	int new_ci = static_cast<int>(new_h / 2);
	int old_ci = static_cast<int>(src.shape()[0] / 2);
	int new_cj = static_cast<int>(new_w / 2);
	int old_cj = static_cast<int>(src.shape()[1] / 2);

	for(int i = 0; i < new_h; i++)
		for (int j = 0; j < new_w; j++)
		{
			int di = i - new_ci;
			int dj = j - new_cj;
			float ri = di * a_cos - dj * a_sin;
			float rj = di * a_sin + dj * a_cos;
			int oi = static_cast<int>(std::roundf(ri)) + old_ci;
			int oj = static_cast<int>(std::roundf(rj)) + old_cj;
			if ((oi < 0) || (oi >= src.shape()[0]) || (oj < 0) || (oj >= src.shape()[1]))
				res.at(i, j) = 0.0f;
			else
				res.at(i, j) = src.at(oi, oj);
		}

	return res;
}


float bujo::transform::getTextAngle(const xt::xtensor<float, 2>& src, float max_angle)
{
	float sz = std::sqrtf(static_cast<float>(src.shape()[0] * src.shape()[0] + src.shape()[1] * src.shape()[1]));
	unsigned num_angles = std::min(100, int(std::ceilf(sz * 0.5f)));
	if (!(num_angles & 1))
		num_angles += 1; //make odd number of angles to include angle 0
	unsigned num_offset = std::min(100, int(std::ceilf(sz * 0.5f)));
	auto angles = xt::linspace<float>(-max_angle, max_angle, num_angles);
	auto res1 = bujo::radon::radon(src, angles, 100, bujo::radon::RT_RADON);
	auto res2 = std::get<0>(res1);
	auto res3 = xt::abs(xt::view(res2, xt::all(), xt::range(1, xt::placeholders::_)) - xt::view(res2, xt::all(), xt::range(xt::placeholders::_, -1)));
	auto res4 = xt::sum(res3, 1);

	auto idx = xt::argmax(res4)[0];
	return angles[idx];
}

template<class T>
inline float get_norm_by_offset_(const T& x, int offset)
{
	auto x0 = xt::view(x, xt::range(offset, xt::placeholders::_));
	auto x1 = xt::view(x, xt::range(xt::placeholders::_, -offset));
	return static_cast<float>(xt::mean(xt::abs(x0-x1))[0]);
}

int bujo::transform::getTextLineDelta(const xt::xtensor<float, 2>& src)
{
	//v = numpy.sum(numpy.abs(src[:,1:]-src[:,:-1]), axis=1)
	auto v0 = xt::view(src, xt::all(), xt::range(1, xt::placeholders::_)) -
		xt::view(src, xt::all(), xt::range(xt::placeholders::_, -1));
	auto v = xt::sum(xt::abs(v0), 1);

	int delta = 1;
	bool ascending = true;
	float prev_value = 0.0f;
	while (delta < src.shape()[0])
	{
		float cur_value = get_norm_by_offset_(v, delta);
		if ((cur_value >= prev_value) && (!ascending))
			return delta - 1;
		ascending = (cur_value >= prev_value);
		prev_value = cur_value;
		delta++;
	}
	return -1;
}

xt::xtensor<float, 2> bujo::transform::filterVarianceQuantile(const xt::xtensor<float, 2>& src, unsigned size_w, unsigned size_h, float qw, float qh)
{
	return bujo::filters::filterVarianceQuantileVH(src, size_w, size_h, qw, qh);
}

float bujo::transform::calculateQuantile(const xt::xtensor<float, 2>& src, float quantile)
{
	std::vector<float> buffer(src.size());
	return bujo::util::calculateQuantile(src.cbegin(), src.cend(), quantile, &buffer[0], buffer.size());
}

xt::xtensor<float, 2> bujo::transform::thresholdImage(const xt::xtensor<float, 2>& src, float cutoff)
{
	return xt::cast<float>(xt::greater(src, cutoff));
}

xt::xtensor<float, 2> bujo::transform::coarseImage(const xt::xtensor<float, 2>& src, float scale, float sigma, float cutoff)
{
	return bujo::transform::thresholdImage(
				bujo::filters::filterGaussian2D(
					bujo::transform::resizeImage(src, scale),
					sigma),
				cutoff);
}

std::vector<bujo::splits::RegionSplit> bujo::transform::findVSplits(const xt::xtensor<float, 2>& src, float min_angle, unsigned num_angles, float minimal_abs_split_intensity, float maximal_abs_intersection, float minimal_pct_split)
{
	xt::xtensor<float, 2> tmp = src;

	std::vector<bujo::splits::RegionSplit> res;
	auto angles = xt::concatenate(std::make_tuple(xt::linspace(-pi_f * 0.5f, -min_angle, num_angles / 2 + 1),
		xt::linspace(min_angle, pi_f * 0.5f, num_angles / 2 + 1)));

	float diag_size = std::sqrtf(static_cast<float>(src.shape()[0] * src.shape()[0] + src.shape()[1] * src.shape()[1]));
	unsigned num_offsets = unsigned(std::ceilf(diag_size)*2);
	unsigned window_size = unsigned(std::ceilf(diag_size * 0.4f));
	 
	while (1)
	{
		/*
		auto tmpr1 = bujo::radon::radon(tmp, angles, num_offsets, bujo::radon::RT_RADON);
		auto tmpr2 = bujo::radon::radon(tmp, angles, num_offsets, bujo::radon::RT_RADON_ACCURATE);


		cv::Mat cv1 = bujo::util::xt2cv(std::get<0>(tmpr1), CV_8U);
		cv::Mat cv2 = bujo::util::xt2cv(std::get<0>(tmpr2), CV_8U);
		cv::namedWindow("Radon-optimal", cv::WINDOW_AUTOSIZE);
		cv::imshow("Radon-optimal", cv1);
		cv::namedWindow("Radon-accurate", cv::WINDOW_AUTOSIZE);
		cv::imshow("Radon-accurate", cv2);

		break;*/
		auto splt = bujo::splits::findBestVSplit(tmp, angles, num_offsets, window_size,
			minimal_abs_split_intensity, maximal_abs_intersection, minimal_pct_split);

		//std::cout << splt.desc.direction << ": " << splt.stats.volume_before << " " << splt.stats.volume_inside << " " << splt.stats.volume_after << "\n";
		//std::cout << splt.desc.angle << " " << splt.desc.offset << " " << splt.desc.offset_margin << "\n";

		/*
		float a_x = std::sinf(splt.desc.angle), a_y = -std::cosf(splt.desc.angle);
		float r_offset = -0.5f * tmp.shape()[1] * a_x - 0.5f * tmp.shape()[0] * a_y;
		//float offset = 29.5f; //splt.desc.offset - 5
		float offset = splt.desc.offset;
		int i0 = 0;
		int j0 = static_cast<int>((-r_offset + offset) / a_x);
		int i1 = tmp.shape()[0] - 1;
		int j1 = static_cast<int>((-r_offset + offset - i1 * a_y) / a_x);
		std::cout << r_offset << " " << splt.desc.offset << " " << a_x << " " << 5/a_x << "\n";
		std::cout << "(" << j0 << ", " << i0 << ") - (" << j1 << ", " << i1 << ");\n";
		*/
		if (splt.desc.direction == 0)
			break;
		res.push_back(splt);
		bujo::splits::setRegionValue(tmp, splt.desc, 1, 0.0f);
		/*
		cv::Mat cv1 = bujo::util::xt2cv(tmp, CV_8U);
		cv::line(cv1, cv::Point(j0, i0), cv::Point(j1, i1), cv::Scalar(255));
		cv::namedWindow("Tmp", cv::WINDOW_AUTOSIZE);
		cv::imshow("Tmp", cv1);

		break;*/
	}

	return res;
}

void bujo::transform::setRegionsValue(xt::xtensor<float, 2>& src, std::vector<bujo::splits::RegionSplit> splits, float dsize, float value)
{
	for (unsigned i = 0; i < splits.size(); i++)
		bujo::splits::setRegionValue(src,
			bujo::splits::rescaleSplit(splits[i].desc, dsize), 1, value);
}

std::vector<bujo::curves::Curve> bujo::transform::generateSupportCurves(const xt::xtensor<float, 2>& src,
	unsigned num_curves, float quantile_v, float quantile_h,
	unsigned window_x, unsigned window_y, const bujo::curves::CurveGenerationOptions& options, float optimization_reg_coef)
{
	std::vector<bujo::curves::Curve> supportCurves;
	auto start_points = bujo::curves::selectSupportPoints(src, num_curves, quantile_v, quantile_h);
	supportCurves.reserve(start_points.size());

	std::transform(start_points.cbegin(), start_points.cend(), std::back_inserter(supportCurves),
		[&src, &window_y, &window_x, &options, &optimization_reg_coef](const auto & v)
		{ return bujo::curves::optimizeCurve(src,
			bujo::curves::generateCurve(src, std::get<0>(v), std::get<1>(v), options),
			window_y, window_x, optimization_reg_coef); });
	return supportCurves;
}

std::vector<bujo::curves::Curve> bujo::transform::generateAllCurves(const xt::xtensor<float, 2>& src, const std::vector<bujo::curves::Curve>& supportCurves,
	unsigned window_x, unsigned window_y, float min_value, float max_ratio, float optimization_reg_coef)
{
	auto curve_combos = bujo::curves::generateCurveCombinations(src, supportCurves);
	auto combo_values = bujo::curves::calculateCurveCombinationIntegral(src, supportCurves, curve_combos);
	auto curve_candidates = bujo::curves::selectCurveCandidates(combo_values, window_y, min_value, max_ratio);

	float clip_max = static_cast<float>(src.size() - 1);

	std::vector<bujo::curves::Curve> allCurves;
	allCurves.reserve(curve_candidates.size());
	std::transform(curve_candidates.cbegin(), curve_candidates.cend(), std::back_inserter(allCurves),
		[&src, &supportCurves, &curve_combos, &clip_max, &optimization_reg_coef, &window_y, &window_x](const auto & v)
		{ return bujo::curves::clipCurve(src, bujo::curves::optimizeCurve(src,
			bujo::curves::generateCurve(supportCurves, curve_combos[v], 0.0f, clip_max),
			window_y, window_x, optimization_reg_coef), window_y / 2, window_y / 2); });

	return allCurves;
}

std::vector<Word> bujo::transform::generateWords(const xt::xtensor<float, 2>& src, const bujo::curves::Curve& curve, unsigned max_offset, float reg_coef, unsigned window, float cutoff_value)
{
	auto h = bujo::curves::getCurveHeight(src, curve, max_offset, reg_coef);
	auto line = bujo::curves::extractCurveRegion(src, curve, std::get<0>(h), std::get<1>(h));
	auto ranges = bujo::curves::locateWordsInLine(line, window, cutoff_value);
	std::vector<Word> res;
	res.reserve(ranges.size());
	for (int i = 0; i < ranges.size(); i++)
	{
		Word wrd;
		wrd.curve = bujo::curves::extractCurve(curve, ranges[i]);
		wrd.neg_offset = static_cast<float>(std::get<0>(h));
		wrd.pos_offset = static_cast<float>(std::get<1>(h));
		res.push_back(std::move(wrd));
	}
	return std::move(res);
}

std::vector<Word> bujo::transform::generateWords(const xt::xtensor<float, 2>& src,
	const bujo::curves::Curve& curve, unsigned pos_offset, unsigned neg_offset,
	unsigned filter_size, unsigned window, const bujo::curves::WordDetectionOptions& options)
{
	auto line = bujo::curves::extractCurveRegion(src, curve, neg_offset, pos_offset);

	auto ranges = bujo::curves::locateWordsInLine(line, window, filter_size, options);
	std::vector<Word> res;
	res.reserve(ranges.size());

	for (int i = 0; i < ranges.size(); i++)
	{
		float p0 = std::get<0>(ranges[i]);
		float p1 = std::get<1>(ranges[i]);
		//std::cout << "(" << p0 << ", " << p1 << ") ";
		/*
		xt::xtensor<float, 1> tmpt;
		tmpt.resize({ 2 });
		tmpt.at(0) = p0; tmpt.at(1) = p1;
		auto tmpr = xt::interp(tmpt, curve.len_param, curve.x_value);
		std::cout << "(" << tmpr[0] << ", " << tmpr[1] << ") ";
		*/
		
		Word wrd;
		wrd.curve = bujo::curves::extractCurve(curve, p0, p1);
		wrd.neg_offset = static_cast<float>(neg_offset);
		wrd.pos_offset = static_cast<float>(pos_offset);

		res.push_back(std::move(wrd));
	}
	return std::move(res);
}

Word bujo::transform::transformWord(const Word& wrd, float x_offset, float x_factor, float y_offset, float y_factor, float h_factor)
{
	Word res;
	res.curve = bujo::curves::affineTransformCurve(wrd.curve, x_offset, x_factor, y_offset, y_factor);
	res.neg_offset = wrd.neg_offset * h_factor * y_factor;
	res.pos_offset = wrd.pos_offset * h_factor * y_factor;
	return res;
}

xt::xtensor<float, 2> bujo::transform::extractWord(const xt::xtensor<float, 2>& src, const Word& word)
{
	unsigned neg_offset = static_cast<unsigned>(std::ceilf(word.neg_offset));
	unsigned pos_offset = static_cast<unsigned>(std::ceilf(word.pos_offset));
	return bujo::curves::extractCurveRegion(src, word.curve, neg_offset, pos_offset);
}
