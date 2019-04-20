#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <exception>
#include <xtensor/xarray.hpp>
#include <algorithm>
#include <src/util/utils.h>
#include <src/radon.h>
#include <chrono>

void dev03()
{
	cv::Mat src0, src1, src2;
	src0 = cv::imread("D:\\Data\\bujo_sample\\20190309_125151.jpg", cv::IMREAD_COLOR);
	if (src0.empty()) // Check for invalid input
		throw std::runtime_error("Could not open file with test image!");
	cv::resize(src0, src1, cv::Size(), 0.1, 0.1);
	cv::cvtColor(src1, src1, cv::COLOR_RGB2GRAY);

	auto t0 = std::chrono::high_resolution_clock::now();

	auto res1 = bujo::radon::radon(bujo::util::cv2xtf(src1), xt::linspace<float>(-3.14f, 3.14f, 400), 100, bujo::radon::RT_RADON_VANILLA);
	
	auto t1 = std::chrono::high_resolution_clock::now();
	
	auto res2 = bujo::radon::radon(bujo::util::cv2xtf(src1), xt::linspace<float>(-3.14f, 3.14f, 400), 100, bujo::radon::RT_RADON);
	
	auto t2 = std::chrono::high_resolution_clock::now();

	std::cout << std::chrono::duration<double>(t1 - t0).count() << " " << std::chrono::duration<double>(t2 - t1).count() << "\n";
	auto rdiff = std::get<0>(res1) - std::get<0>(res2);
	std::cout << xt::amin(rdiff)[0] << " " << xt::amax(rdiff)[0] << "\n";

	auto x1 = std::get<0>(res2);
	std::cout << xt::mean(x1)[0] << "\n" << xt::amin(x1)[0] << "\n" << xt::amax(x1)[0] << "\n\n";
	std::cout << x1.shape()[0] << " - " << x1.shape()[1] << "\n\n";
	std::cout << std::sinf(1.5f);

	src2 = bujo::util::xt2cv(x1/xt::amax(x1)[0], CV_8U);

	cv::namedWindow("Src2", cv::WINDOW_AUTOSIZE); 
	cv::imshow("Src2", src2); 

	cv::waitKey(0);
}