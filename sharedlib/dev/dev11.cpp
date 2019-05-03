#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <exception>
#include <xtensor/xarray.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>
#include <xtensor/xsort.hpp>
#include <xtensor/xrandom.hpp>
#include <algorithm>
#include <chrono>
#include <src/util/utils.h>
#include <src/transform.h>
#include <src/filters.h>
#include <src/extremum.h>
#include <src/curves.h>
#include "devutils.h"
#include <xtensor/xio.hpp>

void dev11()
{
	cv::Mat cv0, cv1;
	cv0 = cv::imread("D:\\Data\\bujo_sample\\20190309_125151.jpg", cv::IMREAD_COLOR);
	//cv0 = cv::imread("D:\\Data\\bujo_sample\\test_rot30.jpg", cv::IMREAD_COLOR);
	cv::resize(cv0, cv0, cv::Size(), 0.1, 0.1);
	cv::cvtColor(cv0, cv0, cv::COLOR_RGB2GRAY);
	if (cv0.empty()) // Check for invalid input
		throw std::runtime_error("Could not open file with test image!");

	auto src0 = bujo::util::cv2xt(cv0);

	auto t0 = std::chrono::system_clock::now();

	float textAngle = bujo::transform::getTextAngle(src0);
	auto src1 = bujo::transform::rotateImage(src0, -textAngle);
	int textLineDelta = bujo::transform::getTextLineDelta(src1);
	auto src2 = bujo::transform::filterVarianceQuantile(src1, textLineDelta / 2, textLineDelta / 2, 0.5f, 0.5f);
	float textCutoff = bujo::transform::calculateQuantile(src2, 0.9f) * 0.5f;
	auto src3 = bujo::transform::thresholdImage(src2, textCutoff);
	auto src4 = bujo::transform::coarseImage(src3, 0.25f, 0.5f, 0.2f);

	auto splits = bujo::transform::findVSplits(src4, 0.5f, 50, 10.0f, 2.0f, 0.05f);

	auto src5 = src2;
	bujo::transform::setRegionsValue(src5, splits, 4.0f, 0.0f);
	auto src6 = bujo::filters::filterLocalMax2DV(src5, textLineDelta, 1, textCutoff);

	auto supportCurves = bujo::transform::generateSupportCurves(src6, 6, 0.5f, 0.5f, 25, textLineDelta);
	auto allCurves = bujo::transform::generateAllCurves(src6, supportCurves, 25, textLineDelta);

	auto t1 = std::chrono::system_clock::now();

	cv1 = bujo::util::xt2cv(src6, CV_8U);
	for (int i = 0; i < allCurves.size(); i++)
	{
		plot(cv1, allCurves[i]);
		auto curveHeight = bujo::curves::getCurveHeight(src6, allCurves[i], textLineDelta * 3, 0.001f);
		std::cout << i << ": " << std::get<0>(curveHeight) << " " << std::get<1>(curveHeight) << "\n";
	}
	std::cout << "\n\n";


	std::cout << "Elapsed " << std::chrono::duration<float>(t1 - t0).count() << "s.\n\n";
	std::cout << "Text angle is " << textAngle << " radians\n";
	std::cout << "Text line delta is " << textLineDelta << " pixels\n";
	std::cout << "Cutoff is " << textCutoff << "\n";
	std::cout << src6.shape()[0] << " " << src6.shape()[1] << "\n";

	cv::namedWindow("Src", cv::WINDOW_AUTOSIZE);
	cv::imshow("Src", cv1);

	cv::waitKey(0);
}