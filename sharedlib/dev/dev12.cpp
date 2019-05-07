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
#include <src/util/cv_ops.h>
#include <src/transform.h>
#include <src/filters.h>
#include <src/extremum.h>
#include <src/curves.h>
#include "devutils.h"
#include <xtensor/xio.hpp>

void dev12()
{
	cv::Mat cv0, cv1, cvi;
	cv0 = cv::imread("D:\\Data\\bujo_sample\\20190309_125151.jpg", cv::IMREAD_COLOR);
	//cv0 = cv::imread("D:\\Data\\bujo_sample\\test_rot30.jpg", cv::IMREAD_COLOR);
	cv::cvtColor(cv0, cv0, cv::COLOR_RGB2GRAY);
	cv::resize(cv0, cvi, cv::Size(), 0.5, 0.5);
	auto img0 = bujo::util::cv2xt(cvi);
	cv::resize(cv0, cv0, cv::Size(), 0.1, 0.1);
	if (cv0.empty()) // Check for invalid input
		throw std::runtime_error("Could not open file with test image!");

	auto src0 = bujo::util::cv2xt(cv0);
	//auto src0 = bujo::transform::resizeImage(img0, 0.2f);

	auto t0 = std::chrono::system_clock::now();

	float textAngle = bujo::transform::getTextAngle(src0);
	auto src1 = bujo::transform::rotateImage(src0, -textAngle);
	int textLineDelta = bujo::transform::getTextLineDelta(src1);
	auto src2 = bujo::transform::filterVarianceQuantile(src1, textLineDelta / 2, textLineDelta / 2, 0.5f, 0.5f);
	float textCutoff = bujo::transform::calculateQuantile(src2, 0.9f) * 0.5f;
	auto src3 = bujo::transform::thresholdImage(src2, textCutoff);
	auto src4 = bujo::transform::coarseImage(src3, 0.25f, 0.5f, 0.2f);

	auto splits = bujo::transform::findVSplits(src4, 0.5f, 50, 5.0f, 2.0f, 0.03f);

	auto src5 = src2;
	bujo::transform::setRegionsValue(src5, splits, 4.0f, 0.0f);
	auto src6 = bujo::filters::filterLocalMax2DV(src5, textLineDelta, 1, textCutoff);

	auto supportCurves = bujo::transform::generateSupportCurves(src6, 6, 0.5f, 0.5f, 25, textLineDelta);
	auto allCurves = bujo::transform::generateAllCurves(src6, supportCurves, 25, textLineDelta);

	std::vector<std::vector<bujo::transform::Word>> allWords;
	allWords.reserve(allCurves.size());
	std::transform(allCurves.cbegin(), allCurves.cend(), std::back_inserter(allWords),
		[&src5, &textLineDelta, &textCutoff](const auto & v) {
			return bujo::transform::generateWords(src5 > textCutoff, v, textLineDelta * 2, 0.1f, 5, 0.1f);
		});

	auto t1 = std::chrono::system_clock::now();
	
	std::cout << src1.shape()[0] << " " << src1.shape()[1] << "\n";
	std::cout << src5.shape()[0] << " " << src5.shape()[1] << "\n";
	for (int i = 0; i < allWords.size(); i++)
	{
		std::cout << i << ": " << allWords[i].size() << "\n";
	}
	//auto tmp0 = bujo::curves::affineTransformCurve(allCurves[36], textLineDelta / 2 - 1, 5.0f, textLineDelta / 2 - 1, 5.0f);
	//auto tmp1 = bujo::curves::extractCurveRegion(img0, tmp0, 25, 25);
	auto tmp0 = bujo::transform::transformWord(allWords[35][1], static_cast<float>(textLineDelta / 2), 5.0f,
		static_cast<float>(textLineDelta / 2)-1, 5.0f, 1.25f);
	auto tmp1 = bujo::transform::extractWord(img0, tmp0);
	//cv1 = bujo::util::xt2cv(tmp1, CV_8U);
	cv1 = bujo::util::xt2cv(src5 > textCutoff, CV_8U);

	std::cout << "Elapsed " << std::chrono::duration<float>(t1 - t0).count() << "s.\n\n";
	std::cout << "Text angle is " << textAngle << " radians\n";
	std::cout << "Text line delta is " << textLineDelta << " pixels\n";
	std::cout << "Cutoff is " << textCutoff << "\n";
	std::cout << src6.shape()[0] << " " << src6.shape()[1] << "\n";

	cv::namedWindow("Src", cv::WINDOW_AUTOSIZE);
	cv::imshow("Src", cv1);

	cv::waitKey(0);
}