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

void test_xtensor_interp()
{
	xt::xtensor<float, 1> src_x({ 1.0f, 9.0f});

	std::cout << xt::interp(xt::arange(-1, 11), src_x, src_x) << "\n\n";
}

void test_xtensor_union()
{
	xt::xtensor<float, 1> x1({ 1.0f, 5.0f, 9.0f }), x2({ -1.0f, 5.0f, 11.0f });
	xt::xtensor<float, 1> x3 = xt::concatenate(std::make_tuple(x1, x2));
	auto x = xt::unique(xt::sort(x3));

	std::cout << x << "\n\n";
}

void test_xtensor_copy()
{
	xt::xtensor<float, 1> x1({ 1.0f, 5.0f, 9.0f }), x2({ -1.0f, 5.0f, 11.0f });
	x2 = x1;
	x2[0] = 15.0f;
	std::cout << x1 << "\n";
	std::cout << x2 << "\n";

}

void dev10()
{
	//test_xtensor_interp();
	//test_xtensor_union();
	//test_xtensor_copy();

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

	std::vector<bujo::curves::Curve> supportCurves;
	auto start_points = bujo::curves::selectSupportPoints(src6, 6, 0.5f, 0.5f);
	supportCurves.reserve(start_points.size());

	std::transform(start_points.cbegin(), start_points.cend(), std::back_inserter(supportCurves),
		[&src6, &textLineDelta](const auto & v)
		{ return bujo::curves::optimizeCurve(src6,
			bujo::curves::generateCurve(src6, std::get<0>(v), std::get<1>(v), bujo::curves::CurveGenerationOptions()),
			textLineDelta, 25, 0.1f); });

	auto curve_combos = bujo::curves::generateCurveCombinations(src6, supportCurves);
	auto combo_values = bujo::curves::calculateCurveCombinationIntegral(src6, supportCurves, curve_combos);

	auto t1 = std::chrono::system_clock::now();

	std::cout << combo_values << "\n\n";

	cv1 = bujo::util::xt2cv(src6, CV_8U);
	for (int i = 0; i < supportCurves.size(); i++)
	{
		plot(cv1, supportCurves[i]);
		std::cout << supportCurves[i].len_param.size() << " " << supportCurves[i].x_value.size() << "\n";
	}


	std::cout << "Elapsed " << std::chrono::duration<float>(t1 - t0).count() << "s.\n\n";
	std::cout << "Text angle is " << textAngle << " radians\n";
	std::cout << "Text line delta is " << textLineDelta << " pixels\n";
	std::cout << "Cutoff is " << textCutoff << "\n";
	std::cout << src6.shape()[0] << " " << src6.shape()[1] << "\n";

	cv::namedWindow("Src", cv::WINDOW_AUTOSIZE);
	cv::imshow("Src", cv1);

	cv::waitKey(0);
}