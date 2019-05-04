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
#include <src/detector.h>
#include <src/util/utils.h>
#include "devutils.h"
#include <xtensor/xio.hpp>

void dev13()
{
	cv::Mat cv0, cv1, cvi;
	cv0 = cv::imread("D:\\Data\\bujo_sample\\20190309_125151.jpg", cv::IMREAD_COLOR);
	//cv0 = cv::imread("D:\\Data\\bujo_sample\\test_rot30.jpg", cv::IMREAD_COLOR);
	cv::cvtColor(cv0, cv0, cv::COLOR_RGB2GRAY);
	cv::resize(cv0, cv0, cv::Size(), 0.5, 0.5);
	if (cv0.empty()) // Check for invalid input
		throw std::runtime_error("Could not open file with test image!");


	auto img0 = bujo::util::cv2xt(cv0);

	auto t0 = std::chrono::system_clock::now();
	
	bujo::detector::Detector det;
	det.loadImage(img0, 0.2f);
	det.updateRegionAuto();
	det.selectSupportCurvesAuto(6, 25);
	det.detectWords(25, 5);
	
	auto t1 = std::chrono::system_clock::now();

	for (int i = 0; i < det.numLines(); i++)
	{
		std::cout << i << ": " << det.numWords(i) << "\n";
	}
	//cv1 = bujo::util::xt2cv(det.extractWord(0, 2, 1.25f), CV_8U);
	cv1 = bujo::util::xt2cv(det.mainImage() > det.textCutoff(), CV_8U);

	std::cout << "Elapsed " << std::chrono::duration<float>(t1 - t0).count() << "s.\n\n";
	std::cout << "Text angle is " << det.textAngle() << " radians\n";
	std::cout << "Text line delta is " << det.textDelta() << " pixels\n";

	cv::namedWindow("Src", cv::WINDOW_AUTOSIZE);
	cv::imshow("Src", cv1);

	cv::waitKey(0);
}