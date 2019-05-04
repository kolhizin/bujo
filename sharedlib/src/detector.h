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
		class Detector
		{
			xt::xtensor<float, 2> originalImg_, alignedOriginalImg_;

			xt::xtensor<float, 2> sourceImg_, alignedImg_;
			xt::xtensor<float, 2> filteredImg_;
			xt::xtensor<float, 2> coarseImg_;

			xt::xtensor<float, 2> usedImg_, textImg_;

			std::vector<bujo::curves::Curve> supportCurves_, allCurves_;
			std::vector<std::vector<bujo::transform::Word>> words_;

			unsigned kernel_v_, kernel_h_;

			float angle_, textCutoff_, scale_;
			unsigned textLineDelta_;
			void clear_();
		public:
			void loadImage(const xt::xtensor<float, 2>& src, float scale = 1.0f, const FilteringOptions &options = FilteringOptions());

			void updateRegionAuto(float min_angle = 0.5f, unsigned num_angles = 50, float minimal_abs_split_intensity = 10.0f,
				float maximal_abs_intersection = 2.0f, float minimal_pct_split = 0.05f);

			void selectSupportCurvesAuto(unsigned num_curves, unsigned window, float quantile_v = 0.5f, float quantile_h = 0.5f);

			void detectWords(unsigned curve_window, unsigned word_window, float word_cutoff=0.1f, float reg_coef=0.1f);

			inline float textAngle() const { return angle_; }
			inline unsigned textDelta() const { return static_cast<unsigned>(textLineDelta_ / scale_); }
			inline float textCutoff() const { return textCutoff_; }

			const xt::xtensor<float, 2>& mainImage() const { return usedImg_; }
			const xt::xtensor<float, 2>& coarseImage() const { return coarseImg_; }
			const xt::xtensor<float, 2>& textImage() const { return textImg_; }


			inline unsigned numLines() const { return words_.size(); }
			inline unsigned numWords(unsigned line) const { return words_.at(line).size(); }
			inline unsigned numWords() const
			{
				unsigned res = 0;
				for (unsigned i = 0; i < words_.size(); i++)
					res += words_[i].size();
				return res;
			}
			const bujo::curves::Curve& getLine(unsigned lineId) const { return allCurves_.at(lineId); }
			const bujo::curves::Curve& getSupportLine(unsigned supportLineId) const { return supportCurves_.at(supportLineId); }
			xt::xtensor<float, 2> extractLine(unsigned lineId, unsigned neg_height, unsigned pos_height) const;
			xt::xtensor<float, 2> extractWord(unsigned lineId, unsigned wordId, float height_scale=1.0f) const;
		};
	}
}