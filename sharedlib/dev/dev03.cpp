#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <exception>

void dev03()
{
	cv::Mat src0, src1, src2;
	src0 = cv::imread("D:\\Data\\bujo_sample\\20190309_125151.jpg", cv::IMREAD_COLOR);
	if (src0.empty()) // Check for invalid input
		throw std::runtime_error("Could not open file with test image!");
	cv::resize(src0, src1, cv::Size(), 0.1, 0.1);
	cv::cvtColor(src1, src1, cv::COLOR_RGB2GRAY);

	cv::namedWindow("Src1", cv::WINDOW_AUTOSIZE); 
	cv::imshow("Src1", src1); 

	cv::waitKey(0);
}