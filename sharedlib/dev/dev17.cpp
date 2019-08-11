#include <opencv2/opencv.hpp>
#include <exception>
#include <iostream>
#include <chrono>
#include <src/detector.h>
#include <src/util/cv_ops.h>
#include "devutils.h"
#include <sstream>

//goal of this file is to read alphabet
void dev16()
{
	cv::Mat cv0, cv1;
	cv0 = cv::imread("D:\\Data\\alphabet\\20190811_151351.jpg", cv::IMREAD_COLOR);
	//cv0 = cv::imread("D:\\Data\\alphabet\\20190811_151402.jpg", cv::IMREAD_COLOR);
	cv::cvtColor(cv0, cv0, cv::COLOR_RGB2GRAY);
	cv::resize(cv0, cv0, cv::Size(), 0.5, 0.5);
	if (cv0.empty()) // Check for invalid input
		throw std::runtime_error("Could not open file with test image!");


	auto img0 = bujo::util::cv2xt(cv0);

	auto t0 = std::chrono::system_clock::now();

	bujo::detector::Detector det;
	bujo::detector::FilteringOptions opts;
	bujo::curves::WordDetectionOptions wopts;
	opts.cutoff_quantile = 0.95f;
	wopts.sigma_h = 7.0f; //default is 10
	wopts.cutoff_ratio = 0.5f;
	det.loadImage(img0, 0.2f);
	det.detectAngle(0.7f);
	det.alignImages();
	det.filterImages(opts);
	det.updateRegionAuto(1.2f, 100, 10.0f, 0.0f, 0.05f);

	cv1 = bujo::util::xt2cv(det.mainImage() > det.textCutoff(), CV_8U);
	cv::namedWindow("Region", cv::WINDOW_AUTOSIZE);
	cv::imshow("Region", cv1);

	det.selectSupportCurvesAuto(6, 25);
	det.detectLines(25);
	//det.detectWords(5, 4, 0.1f, wopts);
	for (int i = 0; i < det.numLines(); i++)
	{
		det.detectWords(i, 5, 4, 0.1f, wopts);
	}

	auto t1 = std::chrono::system_clock::now();

	cv::imwrite("TempResult/img_aligned_original.jpg", bujo::util::xt2cv(det.alignedOriginalImage(), CV_8U));
	for (int i = 0; i < static_cast<int>(det.numLines()); i++)
	{
		std::cout << i << ": " << det.numWords(i) << "\n";
		std::stringstream ss;
		ss << "TempResult/full_line_" << std::setfill('0') << std::setw(2) << i << ".jpg";
		cv::imwrite(ss.str(), bujo::util::xt2cv(det.extractLine(i, 25, 25), CV_8U));
		for (int j = 0; j < static_cast<int>(det.numWords(i)); j++)
		{
			std::stringstream ss;
			ss << "TempResult/line_" << std::setfill('0') << std::setw(2) << i <<
				"_word_" << std::setfill('0') << std::setw(2) << j << ".jpg";
			cv::imwrite(ss.str(), bujo::util::xt2cv(det.extractWord(i, j, 1.3f), CV_8U));
		}
	}


	cv1 = bujo::util::xt2cv(det.mainImage() > det.textCutoff(), CV_8U);
	//for (int i = 0; i < static_cast<int>(det.numSupportLines()); i++)
	//	plot(cv1, det.getSupportLineCurve(i));
	for (int i = 0; i < static_cast<int>(det.numLines()); i++)
		plot(cv1, det.getLineCurve(i));


	std::cout << "Elapsed " << std::chrono::duration<float>(t1 - t0).count() << "s.\n\n";
	std::cout << "Text angle is " << det.textAngle() << " radians\n";
	std::cout << "Text line delta is " << det.textDelta() << " pixels\n";
	std::cout << "Text cutoff is " << det.textCutoff() << "\n";

	cv::namedWindow("Src", cv::WINDOW_AUTOSIZE);
	cv::imshow("Src", cv1);

	cv::waitKey(0);
}