#pragma once
#include <xtensor/xtensor.hpp>
#include <vector>

namespace bujo
{
	namespace curves
	{
		struct Curve
		{
			xt::xtensor<float, 1> len_param;
			xt::xtensor<float, 1> x_value, y_value;

			inline bool check() const
			{
				return (len_param.size() == x_value.size()) && (len_param.size() == y_value.size());
			}
			void calculateLenParametrization();
		};


		struct CurveCombination
		{
			int idx1, idx2;
			float alpha, offset;
		};

		namespace integral
		{
			float calcIntegralOverCurve(const xt::xtensor<float, 2>& src, const Curve& curve, float offset = 0.0f);
			xt::xtensor<float, 1> calcIntegralOverCurve(const xt::xtensor<float, 2>& src, const Curve& curve, const xt::xtensor<float, 1> &offsets);
			xt::xtensor<float, 1> calcAccumIntegralOverCurve(const xt::xtensor<float, 2>& src, const Curve& curve, float offset = 0.0f);
			xt::xtensor<float, 2> calcAccumIntegralOverCurve(const xt::xtensor<float, 2>& src, const Curve& curve, const xt::xtensor<float, 1>& offsets);
		}

		struct CurveGenerationOptions
		{
			float max_angle = 1.0f; //in radians
			float rel_step_size = 0.05f; //in percent of image width
			float rel_window_size_x = 0.15f; //in percent of image width
			float rel_window_size_y = 0.15f; //in percent of image height
			unsigned num_radon_angles = 51; //odd to include zero
			unsigned num_radon_offsets = 50;
			float angle_regularization_coef = 0.1f;
			float angle_regularization_power = 0.25f;
			bool check_angle_fields = false;
		};


		struct WordDetectionOptions
		{
			float cutoff_min = 0.8f; //cutoff on maximum value of local minimum for word separation (as pct of max)
			float cutoff_ratio = 0.5f; //cutoff on offseted ratio of separation point and maximum inside specified window
			float cutoff_offset = 0.1f; //offset to move separation point (as pct of max)
			float sigma_h = 7.0f, sigma_v = 2.0f; //for blob-detection parameters of gaussian smoothing
			float cutoff_word_std = 0.05f; //cutoff for elimination of non-words

			float offset_sigma = 3.0f;
			float offset_reg_lo = 8.0f, offset_reg_hi = 4.0f; //regularization options for selection of word offset
			bool update_word_y_location = true;
		};

		
		std::vector<std::tuple<unsigned, unsigned>> selectSupportPoints(const xt::xtensor<float, 2>& src,
			unsigned num_points, float quantile_v, float quantile_h);

		xt::xtensor<float, 2> extractCurveRegion(const xt::xtensor<float, 2>& src, const Curve& curve, unsigned dNeg, unsigned dPos);
		std::tuple<xt::xtensor<float, 1>, xt::xtensor<float, 1>> getDenseXY(const Curve& curve);

		Curve generateCurve(const xt::xtensor<float, 2>& src, int i0, int j0, const CurveGenerationOptions& options);
		Curve optimizeCurveBinarySplit(const xt::xtensor<float, 2>& src, const Curve& curve, unsigned max_offset_y, unsigned min_window_x, float reg_coef);
		Curve optimizeCurve(const xt::xtensor<float, 2>& src, const Curve& curve, unsigned max_offset_y, unsigned min_window_x, float reg_coef);
		//Curve optimizeCurveLocal(const xt::xtensor<float, 2>& src, const Curve& curve, int max_offset_y, float quantile, float reg_coef);
		Curve reparamByLength(const Curve& curve, unsigned numPoints);
		Curve reparamByX(const Curve& curve, unsigned numPoints);

		Curve extractCurve(const Curve& src, float p0, float p1);
		inline Curve extractCurve(const Curve& src, const std::tuple<float, float> &param)
		{
			return extractCurve(src, std::get<0>(param), std::get<1>(param));
		}
		Curve affineTransformCurve(const Curve& src, float x_offset, float x_scale, float y_offset, float y_scale);
		Curve shiftCurve(const Curve& src, float yOffset, float clampMin, float clampMax);
		Curve interpolateCurves(const Curve& src1, const Curve& src2, float alpha);
		Curve clipCurve(const xt::xtensor<float, 2>& src, const Curve& curve, unsigned window, unsigned trim);

		std::vector<CurveCombination> generateCurveCombinations(const xt::xtensor<float, 2>& src, const std::vector<Curve> &supportCurves);
		float calculateCurveCombinationIntegral(const xt::xtensor<float, 2>& src, const std::vector<Curve>& supportCurves,
			const CurveCombination& combination);
		xt::xtensor<float, 1> calculateCurveCombinationIntegral(const xt::xtensor<float, 2>& src, const std::vector<Curve>& supportCurves,
			const std::vector<CurveCombination>& combination);
		std::vector<unsigned> selectCurveCandidates(const xt::xtensor<float, 1>& vals, unsigned window, float min_value, float max_ratio);
		Curve generateCurve(const std::vector<Curve>& supportCurves, const CurveCombination& curveCombination, float clipMin, float clipMax);

		std::tuple<unsigned, unsigned> getCurveHeight(const xt::xtensor<float, 2>& src, const Curve& curve, unsigned max_offset, float reg_coef);

		std::vector<std::tuple<float, float>> locateWordsInLine(const xt::xtensor<float, 2>& srcLine, unsigned window, float min_value);//legacy version
		std::vector<std::tuple<float, float, float>> locateWordsInLine(const xt::xtensor<float, 2>& srcLine, unsigned window, unsigned filter_size, float reg_center = 0.5f, const WordDetectionOptions &options = WordDetectionOptions());

		unsigned locateWordOffset(const xt::xtensor<float, 2>& srcWord, float reg_center, const WordDetectionOptions& options = WordDetectionOptions());
	}
}