#include "detector.h"
#include "filters.h"

using namespace bujo::detector;

void bujo::detector::Detector::clear_()
{
	supportCurves_.clear();
	words_.clear();
	allCurves_.clear();
}

void bujo::detector::Detector::loadImage(const xt::xtensor<float, 2>& src, float scale, const FilteringOptions& options)
{
	clear_();

	originalImg_ = src;

	sourceImg_ = bujo::transform::resizeImage(originalImg_, scale);
	scale_ = static_cast<float>(sourceImg_.shape()[0])/originalImg_.shape()[0];

	angle_ = bujo::transform::getTextAngle(sourceImg_);
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

void bujo::detector::Detector::detectWords(unsigned curve_window, unsigned word_window, float word_cutoff, float reg_coef)
{
	words_.clear();
	allCurves_.clear();
	allCurves_ = bujo::transform::generateAllCurves(textImg_, supportCurves_, curve_window, textLineDelta_);
	auto tmp = usedImg_ > textCutoff_;

	words_.reserve(allCurves_.size());
	std::transform(allCurves_.cbegin(), allCurves_.cend(), std::back_inserter(words_),
		[this, &tmp, &reg_coef, &word_cutoff, &word_window](const auto & v) {
			return bujo::transform::generateWords(tmp, v, textLineDelta_ * 2, word_cutoff, word_window, reg_coef);
		});
}

xt::xtensor<float, 2> bujo::detector::Detector::extractWord(unsigned lineId, unsigned wordId, float height_scale) const
{
	auto tmp0 = bujo::transform::transformWord(words_.at(lineId).at(wordId),
		static_cast<float>(kernel_h_), 1.0f / scale_, static_cast<float>(kernel_v_-1), 1.0f / scale_, height_scale);
	return bujo::transform::extractWord(alignedOriginalImg_, tmp0);
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
		res.at(i, 0) = x_value / (usedImg_.shape()[1]-1);
		res.at(i, 1) = y_value / (usedImg_.shape()[0]-1);
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