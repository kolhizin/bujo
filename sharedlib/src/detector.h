#pragma once
#include <xtensor/xtensor.hpp>
#include <vector>
#include "transform.h"

namespace bujo
{
	namespace detector
	{
		struct FilteringOptions
		{
			float quantile_v = 0.5f, quantile_h = 0.5f;
			float kernel_v_factor = 0.5f, kernel_h_factor = 0.5f;
			float cutoff_quantile = 0.9f, cutoff_coef = 0.5f;
			float coarse_scale = 0.25f, coarse_sigma = 0.5f, coarse_cutoff = 0.2f;
		};

		struct WordOptions
		{
			float cutoff_min = 0.8f; //cutoff on maximum value of local minimum for word separation (as pct of max)
		};
		class Detector
		{
			xt::xtensor<float, 2> originalImg_, alignedOriginalImg_;

			xt::xtensor<float, 2> sourceImg_, alignedImg_;
			xt::xtensor<float, 2> filteredImg_;
			xt::xtensor<float, 2> coarseImg_;

			xt::xtensor<float, 2> usedImg_, textImg_;

			std::vector<bujo::curves::Curve> supportCurves_, allCurves_;
			std::vector<std::vector<bujo::transform::Word>> words_;

			unsigned kernel_v_ = 0, kernel_h_ = 0;

			float angle_ = 0.0f, textCutoff_ = 0.0f, scale_ = 1.0f;
			unsigned textLineDelta_ = 0;
			void clear_();
		public:
			void loadImage(const xt::xtensor<float, 2>& src, float scale = 1.0f, float max_text_angle = 1.57f, const FilteringOptions &options = FilteringOptions());

			void updateRegionAuto(float min_angle = 0.5f, unsigned num_angles = 50, float minimal_abs_split_intensity = 10.0f,
				float maximal_abs_intersection = 2.0f, float minimal_pct_split = 0.05f);

			void selectSupportCurvesAuto(unsigned num_curves, unsigned window, float quantile_v = 0.5f, float quantile_h = 0.5f, float reg_coef = 0.5f,
				const bujo::curves::CurveGenerationOptions &options=bujo::curves::CurveGenerationOptions());

			void detectWords(unsigned curve_window, unsigned word_window, float reg_coef, float word_cutoff);
			void detectWords(unsigned curve_window, unsigned word_window, float reg_coef = 0.1f, const bujo::curves::WordDetectionOptions& options= bujo::curves::WordDetectionOptions());

			inline float textAngle() const { return angle_; }
			inline unsigned textDelta() const { return static_cast<unsigned>(textLineDelta_ / scale_); }
			inline float textCutoff() const { return textCutoff_; }


			const xt::xtensor<float, 2>& alignedOriginalImage() const { return alignedOriginalImg_; }
			const xt::xtensor<float, 2>& mainImage() const { return usedImg_; }
			const xt::xtensor<float, 2>& filteredImage() const { return filteredImg_; }
			const xt::xtensor<float, 2>& coarseImage() const { return coarseImg_; }
			const xt::xtensor<float, 2>& textImage() const { return textImg_; }


			inline unsigned numLines() const { return static_cast<unsigned>(words_.size()); }
			inline unsigned numSupportLines() const { return static_cast<unsigned>(supportCurves_.size()); }
			inline unsigned numWords(unsigned line) const { return static_cast<unsigned>(words_.at(line).size()); }
			inline unsigned numWords() const
			{
				unsigned res = 0;
				for (unsigned i = 0; i < words_.size(); i++)
					res += static_cast<unsigned>(words_[i].size());
				return res;
			}
			const bujo::curves::Curve& getLineCurve(unsigned lineId) const { return allCurves_.at(lineId); }
			const bujo::curves::Curve& getSupportLineCurve(unsigned supportLineId) const { return supportCurves_.at(supportLineId); }

			inline std::tuple<float, float> getWordHeight(unsigned lineId, unsigned wordId) const
			{
				const auto& v = words_.at(lineId).at(wordId);
				return std::make_tuple(-v.neg_offset / (alignedImg_.shape()[0]), v.pos_offset / (alignedImg_.shape()[0]));
			}
			xt::xtensor<float, 2> getWord(unsigned lineId, unsigned wordId, const xt::xtensor<float, 1> &locs) const;
			xt::xtensor<float, 2> getLine(unsigned lineId, const xt::xtensor<float, 1>& locs) const;
			xt::xtensor<float, 2> getSupportLine(unsigned lineId, const xt::xtensor<float, 1>& locs) const;
			xt::xtensor<float, 2> extractLine(unsigned lineId, unsigned neg_height, unsigned pos_height) const;
			xt::xtensor<float, 2> extractWord(unsigned lineId, unsigned wordId, float height_scale=1.0f) const;

			/*
			x-coord is from 0 to 1, y-coord is from -1 (bottom) to 1 (top), both dimenstions may exceed limits
			returns relative coordinates (0 to 1), where (0,0) is left upper corner
			*/
			xt::xtensor<float, 2> wordCoordinates(unsigned lineId, unsigned wordId, const xt::xtensor<float, 2>& localCoord) const;
		};
	}
}