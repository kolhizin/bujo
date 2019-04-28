#pragma once
#include <opencv2/opencv.hpp>
#include <xtensor/xtensor.hpp>

void plot2d(cv::Mat &img, xt::xtensor<float, 1> x, xt::xtensor<float, 1> y);
void plot2d(cv::Mat& img, xt::xtensor<float, 2> xy);