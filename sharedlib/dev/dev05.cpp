#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <exception>
#include <xtensor/xarray.hpp>
#include <xtensor/xview.hpp>
#include <xtensor/xsort.hpp>
#include <xtensor/xrandom.hpp>
#include <algorithm>
#include <src/util/utils.h>
#include <src/radon.h>
#include <chrono>
#include <src/transform.h>
#include <src/filters.h>

void dev05()
{
	cv::Mat src0, src1, src2, src3, src4, src5;
	src0 = cv::imread("D:\\Data\\bujo_sample\\20190309_125151.jpg", cv::IMREAD_COLOR);
	//src0 = cv::imread("D:\\Data\\bujo_sample\\test_rot30.jpg", cv::IMREAD_COLOR);
	if (src0.empty()) // Check for invalid input
		throw std::runtime_error("Could not open file with test image!");
	cv::resize(src0, src1, cv::Size(), 0.1, 0.1);
	cv::cvtColor(src1, src2, cv::COLOR_RGB2GRAY);

	auto t0 = std::chrono::high_resolution_clock::now();

	float angle = bujo::transform::getTextAngle(src2);
	src3 = bujo::transform::rotateImage(src2, -angle);
	std::cout << src3.rows << " " << src3.cols << "\n";
	int lineDelta = bujo::transform::getTextLineDelta(src3) - 1;
	
	src4 = bujo::transform::applyVarianceCutoff(src3, lineDelta / 2, lineDelta / 2, 0.9);
	src5 = bujo::transform::coarseImage(src4);

	auto t1 = std::chrono::high_resolution_clock::now();

	std::cout << "Elapsed: " << std::chrono::duration<float>(t1 - t0).count() << "s.\n";
	std::cout << "Text angle is " << angle << " radians, line-delta is " << lineDelta << " pixels\n\n";
	//std::cout << "Cutoff is " << unsigned(cutoff) << "\n\n";

	cv::namedWindow("Src", cv::WINDOW_AUTOSIZE);
	cv::imshow("Src", src5);

	cv::waitKey(0);
}