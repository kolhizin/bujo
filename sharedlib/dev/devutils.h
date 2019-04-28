#pragma once
#include <opencv2/opencv.hpp>
#include <xtensor/xtensor.hpp>
#include <src/curves.h>

void plot2d(cv::Mat &img, xt::xtensor<float, 1> x, xt::xtensor<float, 1> y);
void plot2d(cv::Mat& img, xt::xtensor<float, 2> xy);
void plot_aabb(cv::Mat& img, float x0, float x1, float y0, float y1);
void plot_xmark(cv::Mat& img, float x, float y, float size);
void plot(cv::Mat& img, const bujo::curves::Curve& curve);