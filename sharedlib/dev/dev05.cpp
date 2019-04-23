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
//#include <src/radon.h>

//#include <src/transform.h>
//#include <src/filters.h>

void dev05()
{
	cv::Mat cv0, cv1;
	cv0 = cv::imread("D:\\Data\\bujo_sample\\20190309_125151.jpg", cv::IMREAD_COLOR);
	cv::cvtColor(cv0, cv0, cv::COLOR_RGB2GRAY);
	//cv0 = cv::imread("D:\\Data\\bujo_sample\\test_rot30.jpg", cv::IMREAD_COLOR);
	
	if (cv0.empty()) // Check for invalid input
		throw std::runtime_error("Could not open file with test image!");

	auto src0 = bujo::util::cv2xt(cv0);
	cv1 = bujo::util::xt2cv(src0, CV_8U);

	std::cout << src0.shape()[0] << " " << src0.shape()[1] << "\n";

	cv::namedWindow("Src", cv::WINDOW_AUTOSIZE);
	cv::imshow("Src", cv1);

	cv::waitKey(0);
}