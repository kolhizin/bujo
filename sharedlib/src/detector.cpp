#include "detector.h"
#include "filters.h"

using namespace bujo::detector;

void bujo::detector::Detector::clear_()
{
	supportCurves_.clear();
	words_.clear();
	allCurves_.clear();
}

void bujo::detector::Detector::loadImage(const xt::xtensor<float, 2>& src, float scale, float max_text_angle, const FilteringOptions& options)
{
	clear_();

	originalImg_ = src;

	sourceImg_ = bujo::transform::resizeImage(originalImg_, scale);
	scale_ = static_cast<float>(sourceImg_.shape()[0])/originalImg_.shape()[0];

	angle_ = bujo::transform::getTextAngle(sourceImg_, max_text_angle);
	alignedImg_ = bujo::transform::rotateImage(sourceImg_, -angle_);
	alignedOriginalImg_ = bujo::transform::rotateImage(originalImg_, -angle_);
	textLineDelta_ = bujo::transform::getTextLineDelta(alignedImg_);
	kernel_v_ = static_cast<unsigned>(std::floorf(textLineDelta_ * options.kernel_v_factor));
	kernel_h_ = static_cast<unsigned>(std::floorf(textLineDelta_ * options.kernel_h_factor));

	std::cout << textLineDelta_ << " " << kernel_h_ << " " << kernel_v_ << " " << textLineDelta_ / 2 << "\n";

	filteredImg_ = bujo::transform::filterVarianceQuantile(alignedImg_, kernel_h_, kernel_v_,
		options.quantile_h, options.quantile_v);
	textCutoff_ = bujo::transform::calculateQuantile(filteredImg_, options.cutoff_quantile) * options.cutoff_coef;
	
	auto tmp = bujo::transform::thresholdImage(filteredImg_, textCutoff_);
	coarseImg_ = bujo::transform::coarseImage(tmp, options.coarse_scale, options.coarse_sigma, options.coarse_cutoff);

	usedImg_ = filteredImg_;
	textImg_ = bujo::filters::filterLocalMax2DV(usedImg_, textLineDelta_, 2, textCutoff_);
}

void bujo::detector::Detector::updateRegionAuto(float min_angle, unsigned num_angles, float minimal_abs_split_intensity, float maximal_abs_intersection, float minimal_pct_split)
{
	float dsize = static_cast<float>(usedImg_.shape()[0]) / coarseImg_.shape()[0];
	auto splits = bujo::transform::findVSplits(coarseImg_, min_angle, num_angles, minimal_abs_split_intensity,
		maximal_abs_intersection, minimal_pct_split);
	bujo::transform::setRegionsValue(usedImg_, splits, dsize, 0.0f);

	textImg_ = bujo::filters::filterLocalMax2DV(usedImg_, textLineDelta_, 2, textCutoff_);
}

void bujo::detector::Detector::selectSupportCurvesAuto(unsigned num_curves, unsigned window, float quantile_v, float quantile_h, 
	float reg_coef, const bujo::curves::CurveGenerationOptions& options)
{
	supportCurves_ = bujo::transform::generateSupportCurves(textImg_, num_curves,
		quantile_v, quantile_h, window, textLineDelta_, options, reg_coef);
}

void bujo::detector::Detector::detectLines(unsigned curve_window)
{
	words_.clear();
	allCurves_.clear();
	allCurves_ = bujo::transform::generateAllCurves(textImg_, supportCurves_, curve_window, textLineDelta_);
}

void bujo::detector::Detector::detectWords(unsigned word_window, float reg_coef, float word_cutoff)
{
	auto tmp = usedImg_ > textCutoff_;
	words_.clear();
	words_.reserve(allCurves_.size());
	std::transform(allCurves_.cbegin(), allCurves_.cend(), std::back_inserter(words_),
		[this, &tmp, &reg_coef, &word_cutoff, &word_window](const auto & v) {
			auto res = bujo::transform::generateWords(tmp, v, textLineDelta_ * 2, reg_coef, word_window, word_cutoff);
			for (int i = 0; i < res.size(); i++)
				res[i] = bujo::transform::transformWord(res[i],
					static_cast<float>(kernel_h_), 1.0f, static_cast<float>(kernel_v_ - 1), 1.0f, 
					1.0f);
			return std::move(res);
		});
}

void bujo::detector::Detector::detectWords(unsigned word_window, unsigned filter_size, float reg_coef, const bujo::curves::WordDetectionOptions& options)
{
	auto tmp = usedImg_ > textCutoff_;
	float conversion_coef_x = static_cast<float>(alignedImg_.shape()[1]) / (alignedOriginalImg_.shape()[1]);
	float conversion_coef_y = static_cast<float>(alignedImg_.shape()[0]) / (alignedOriginalImg_.shape()[0]);
	words_.clear();
	words_.reserve(allCurves_.size());
	std::transform(allCurves_.cbegin(), allCurves_.cend(), std::back_inserter(words_),
		[this, &tmp, &reg_coef, &options, &word_window, &filter_size, &conversion_coef_x, &conversion_coef_y](const auto & v) {
			curves::Curve ncrv;
			ncrv.x_value = (v.x_value + static_cast<float>(kernel_h_)) / (alignedImg_.shape()[1]) * (alignedOriginalImg_.shape()[1]);
			ncrv.y_value = (v.y_value + static_cast<float>(kernel_v_-1)) / (alignedImg_.shape()[0]) * (alignedOriginalImg_.shape()[0]);
			ncrv.calculateLenParametrization();

			auto h = bujo::curves::getCurveHeight(tmp, v, 2 * textLineDelta_, reg_coef);
			float fneg_offset = static_cast<float>(std::get<0>(h) * (alignedOriginalImg_.shape()[0])) / (alignedImg_.shape()[0]);
			float fpos_offset = static_cast<float>(std::get<1>(h) * (alignedOriginalImg_.shape()[0])) / (alignedImg_.shape()[0]);
			unsigned neg_offset = static_cast<unsigned>(std::ceilf(fneg_offset));
			unsigned pos_offset = static_cast<unsigned>(std::ceilf(fpos_offset));

			auto res = bujo::transform::generateWords(alignedOriginalImg_, ncrv, neg_offset, pos_offset, filter_size, word_window, options);
			for (int i = 0; i < res.size(); i++)
				res[i] = bujo::transform::transformWord(res[i],
					0.0f, conversion_coef_x, 0.0f, conversion_coef_y, 1.0f);
			return std::move(res);
		});
}

xt::xtensor<float, 2> bujo::detector::Detector::extractWord(unsigned lineId, unsigned wordId, float height_scale) const
{
	auto tmp = bujo::transform::transformWord(words_.at(lineId).at(wordId), 0.0f, 1.0f / scale_, 0.0f, 1.0f / scale_, height_scale);
	return bujo::transform::extractWord(alignedOriginalImg_, tmp);
}
xt::xtensor<float, 2> bujo::detector::Detector::wordCoordinates(unsigned lineId, unsigned wordId, const xt::xtensor<float, 2>& localCoord) const
{
	const auto& wrd = words_.at(lineId).at(wordId);
	float max_len = wrd.curve.len_param[wrd.curve.len_param.size() - 1];
	
	xt::xtensor<float, 2> res({ localCoord.shape()[0], 2 });
	xt::xtensor<float, 1> p_tensor;
	p_tensor.resize({ 1 });
	
	for (unsigned i = 0; i < localCoord.shape()[0]; i++)
	{
		float arg_y = localCoord.at(i, 1);
		p_tensor[0] = localCoord.at(i, 0) * max_len;
		float x_value = xt::interp(p_tensor, wrd.curve.len_param, wrd.curve.x_value)[0];
		float y_value = xt::interp(p_tensor, wrd.curve.len_param, wrd.curve.y_value)[0];
		if (arg_y > 0.0f)
			y_value -= wrd.neg_offset * std::fabsf(arg_y);
		else if (arg_y < 0.0f)
			y_value += wrd.pos_offset * std::fabsf(arg_y);
		res.at(i, 0) = x_value / (alignedImg_.shape()[1]);
		res.at(i, 1) = y_value / (alignedImg_.shape()[0]);
	}
	
	return res;
}
xt::xtensor<float, 2> bujo::detector::Detector::getWord(unsigned lineId, unsigned wordId, const xt::xtensor<float, 1>& locs) const
{
	const curves::Curve& crv = words_.at(lineId).at(wordId).curve;
	auto l_param = locs * crv.len_param.at(crv.len_param.size() - 1);
	auto x_value = xt::interp(l_param, crv.len_param, crv.x_value) / (alignedImg_.shape()[1]);
	auto y_value = xt::interp(l_param, crv.len_param, crv.y_value) / (alignedImg_.shape()[0]);
	xt::xtensor<float, 2> res({ x_value.size(), 2 });
	for (unsigned i = 0; i < x_value.size(); i++)
	{
		res.at(i, 0) = x_value[i];
		res.at(i, 1) = y_value[i];
	}
	return res;
}
xt::xtensor<float, 2> bujo::detector::Detector::getLine(unsigned lineId, const xt::xtensor<float, 1>& locs) const
{
	const curves::Curve& crv = allCurves_.at(lineId);
	auto l_param = locs * crv.len_param.at(crv.len_param.size() - 1);
	auto x_value = (xt::interp(l_param, crv.len_param, crv.x_value) + static_cast<float>(kernel_h_)) / (alignedImg_.shape()[1]);
	auto y_value = (xt::interp(l_param, crv.len_param, crv.y_value) + static_cast<float>(kernel_v_-1)) / (alignedImg_.shape()[0]);
	xt::xtensor<float, 2> res({ x_value.size(), 2 });
	for (unsigned i = 0; i < x_value.size(); i++)
	{
		res.at(i, 0) = x_value[i];
		res.at(i, 1) = y_value[i];
	}
	return res;
}
xt::xtensor<float, 2> bujo::detector::Detector::getSupportLine(unsigned lineId, const xt::xtensor<float, 1>& locs) const
{
	const curves::Curve& crv = supportCurves_.at(lineId);
	auto l_param = locs * crv.len_param.at(crv.len_param.size() - 1);
	auto x_value = (xt::interp(l_param, crv.len_param, crv.x_value) + static_cast<float>(kernel_h_)) / (alignedImg_.shape()[1]);
	auto y_value = (xt::interp(l_param, crv.len_param, crv.y_value) + static_cast<float>(kernel_v_ - 1)) / (alignedImg_.shape()[0]);
	xt::xtensor<float, 2> res({ x_value.size(), 2 });
	for (unsigned i = 0; i < x_value.size(); i++)
	{
		res.at(i, 0) = x_value[i];
		res.at(i, 1) = y_value[i];
	}
	return res;
}
xt::xtensor<float, 2> bujo::detector::Detector::extractLine(unsigned lineId, unsigned neg_height, unsigned pos_height) const
{
	auto tmp = bujo::curves::affineTransformCurve(allCurves_.at(lineId),
		static_cast<float>(kernel_h_), 1.0f / scale_,
		static_cast<float>(kernel_v_-1), 1.0f / scale_);
	return bujo::curves::extractCurveRegion(alignedOriginalImg_, tmp, neg_height, pos_height);
}