#include "transform.h"
#include "util/utils.h"
#include "radon.h"
#include "filters.h"
#include <xtensor/xview.hpp>
#include <xtensor/xsort.hpp>

using namespace bujo::transform;
constexpr float pi_f = 3.1415926f;

float bujo::transform::getTextAngle(const cv::Mat& src)
{
	float sz = std::sqrtf(static_cast<float>(src.cols * src.cols + src.rows * src.rows));
	unsigned num_angles = std::min(100, int(std::ceilf(sz * 0.5f)));
	if (!(num_angles & 1))
		num_angles += 1; //make odd number of angles to include angle 0
	unsigned num_offset = std::min(100, int(std::ceilf(sz * 0.5f)));
	auto angles = xt::linspace<float>(-pi_f * 0.5f, pi_f * 0.5f, num_angles);
	auto res1 = bujo::radon::radon(bujo::util::cv2xtf(src), angles, 100, bujo::radon::RT_RADON);
	auto res2 = std::get<0>(res1);
	auto res3 = xt::abs(xt::view(res2, xt::all(), xt::range(1, xt::placeholders::_)) - xt::view(res2, xt::all(), xt::range(xt::placeholders::_, -1)));
	auto res4 = xt::sum(res3, 1);

	auto idx = xt::argmax(res4)[0];
	return angles[idx];
}

template<class T>
inline float get_norm_by_offset_(const T& x, int offset)
{
	auto x0 = xt::view(x, xt::range(offset, xt::placeholders::_), xt::all());
	auto x1 = xt::view(x, xt::range(xt::placeholders::_, -offset), xt::all());
	return static_cast<float>(xt::mean(xt::square(x0-x1))[0]);
}

int bujo::transform::getTextLineDelta(const cv::Mat& src)
{
	if (src.size.dims() != 2)
		throw std::logic_error("Function getTextLineDelta() expects 2d-matrix as src!");
	if ((src.rows <= 0) || (src.cols <= 0))
		throw std::logic_error("Function getTextLineDelta() expects non-trivial 2d-matrix as src!");
	auto x0 = bujo::util::cv2xtf(src);
	int delta = 1;
	bool ascending = true;
	float prev_value = 0.0f;
	while (delta < src.rows)
	{
		float cur_value = get_norm_by_offset_(x0, delta);
		if ((cur_value >= prev_value) && (!ascending))
			return delta;
		ascending = (cur_value >= prev_value);
		prev_value = cur_value;
		delta++;
	}
	return -1;
}

cv::Mat bujo::transform::rotateImage(const cv::Mat& src, float angle)
{
	if (src.size.dims() != 2)
		throw std::logic_error("Function rotateImage() expects 2d-matrix as src!");
	if ((src.rows <= 0) || (src.cols <= 0))
		throw std::logic_error("Function rotateImage() expects non-trivial 2d-matrix as src!");
	cv::Mat res;
	float new_w = std::fabsf(src.cols * std::cosf(angle)) + std::fabsf(src.rows * std::sinf(angle));
	float new_h = std::fabsf(src.cols * std::sinf(angle)) + std::fabsf(src.rows * std::cosf(angle));
	int new_width = static_cast<int>(std::ceilf(new_w));
	int new_height = static_cast<int>(std::ceilf(new_h));
	cv::Mat rot = cv::getRotationMatrix2D(cv::Point2f(src.cols * 0.5f, src.rows * 0.5f),
		-angle * 180.0f / pi_f, 1.0);
	cv::warpAffine(src, res, rot, cv::Size(new_width, new_height));
	return res;
}

cv::Mat bujo::transform::applyVarianceCutoff(const cv::Mat& src, unsigned size_w, unsigned size_h,
	float cutoff_q, float cutoff_coef)
{
	auto xsrc = bujo::util::cv2xtf(src);
	auto xres = bujo::filters::filterVarianceQuantileVH(xsrc, size_w, size_h, 0.5f, 0.5f);
	std::vector<float> buffer(xres.size());
	float cutoff = bujo::util::calculateQuantile(xres.cbegin(), xres.cend(), cutoff_q, &buffer[0], buffer.size());
	auto xres_final = xt::cast<float>(xt::greater(xres, cutoff * cutoff_coef));
	return bujo::util::xt2cv(xres_final, CV_8U);
}

cv::Mat bujo::transform::coarseImage(const cv::Mat& src, float scale, float sigma, float gaussian_threshold)
{
	cv::Mat tmp1, tmp2;
	cv::resize(src, tmp1, cv::Size(), scale, scale);
	cv::threshold(tmp1, tmp2, 0.0, 255.0, cv::THRESH_BINARY);
	cv::GaussianBlur(tmp2, tmp1, cv::Size(20, 20), sigma, sigma);
	//cv::threshold(tmp1, res, gaussian_threshold, 1.0f, cv::THRESH_BINARY);
	return tmp2;
}
