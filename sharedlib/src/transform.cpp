#include "transform.h"
#include "util/utils.h"
#include "radon.h"
#include <xtensor/xview.hpp>
#include <xtensor/xsort.hpp>

using namespace bujo::transform;
constexpr float pi_f = 3.1415926f;

float bujo::transform::getTextAngle(const cv::Mat& src)
{
	float sz = std::sqrtf(static_cast<float>(src.cols * src.cols + src.rows * src.rows));
	unsigned num_angles = std::min(180, int(std::ceilf(sz * 0.5f)));
	unsigned num_offset = std::min(100, int(std::ceilf(sz * 0.5f)));
	auto angles = xt::linspace<float>(-pi_f * 0.5f, pi_f * 0.5f, num_angles);
	auto res1 = bujo::radon::radon(bujo::util::cv2xtf(src), angles, 100, bujo::radon::RT_RADON);
	auto res2 = std::get<0>(res1);
	auto res3 = xt::abs(xt::view(res2, xt::all(), xt::range(1, xt::placeholders::_)) - xt::view(res2, xt::all(), xt::range(xt::placeholders::_, -1)));
	auto res4 = xt::sum(res3, 1);
	auto idx = xt::argmax(res4)[0];
	return angles[idx];
}

cv::Mat bujo::transform::rotateImage(const cv::Mat& src, float angle)
{
	if (src.size.dims() != 2)
		throw std::logic_error("Function rotate() expects 2d-matrix as src!");
	if ((src.rows <= 0) || (src.cols <= 0))
		throw std::logic_error("Function rotate() expects non-trivial 2d-matrix as src!");
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
