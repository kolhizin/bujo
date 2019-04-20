#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <exception>
#include <xtensor/xarray.hpp>
#include <xtensor/xview.hpp>
#include <xtensor/xsort.hpp>
#include <algorithm>
#include <src/util/utils.h>
#include <src/radon.h>
#include <chrono>

void dev04()
{
	cv::Mat src0, src1, src2;
	//src0 = cv::imread("D:\\Data\\bujo_sample\\20190309_125151.jpg", cv::IMREAD_COLOR);
	src0 = cv::imread("D:\\Data\\bujo_sample\\test_rot30.jpg", cv::IMREAD_COLOR);
	if (src0.empty()) // Check for invalid input
		throw std::runtime_error("Could not open file with test image!");
	cv::resize(src0, src1, cv::Size(), 0.1, 0.1);
	cv::cvtColor(src1, src1, cv::COLOR_RGB2GRAY);

	auto t0 = std::chrono::high_resolution_clock::now();

	auto angles = xt::linspace<float>(-3.14f * 0.5f, 3.14f * 0.5f, 400);
	auto tmp = bujo::util::cv2xtf(src1);
	auto res2 = bujo::radon::radon(bujo::util::cv2xtf(src1), angles, 100, bujo::radon::RT_RADON);
	auto res2r = std::get<0>(res2);

	auto t1 = std::chrono::high_resolution_clock::now();

	auto res3 = xt::abs(xt::view(res2r, xt::all(), xt::range(1, xt::placeholders::_)) - xt::view(res2r, xt::all(), xt::range(xt::placeholders::_, -1)));
	auto res4 = xt::sum(res3, 1);
	int idx = xt::argmax(res4)[0];

	std::cout << angles[idx] << "\n";
	src2 = bujo::util::xt2cv(res2r / xt::amax(res2r)[0], CV_8U);


	cv::namedWindow("Src2", cv::WINDOW_AUTOSIZE);
	cv::imshow("Src2", src2);

	cv::waitKey(0);
}