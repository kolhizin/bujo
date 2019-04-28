#include "devutils.h"
#include <exception>
#include <xtensor/xview.hpp>

void plot2d(cv::Mat& img, xt::xtensor<float, 1> x, xt::xtensor<float, 1> y)
{
	if (x.size() != y.size())
		throw std::logic_error("plot2d(x, y): x and y should be the same size!");
	cv::Scalar color(255.0);
	for (int i = 0; i < x.size() - 1; i++)
	{
		cv::Point p1(x.at(i), y.at(i)), p2(x.at(i + 1), y.at(i + 1));
		cv::line(img, p1, p2, color);
	}
}
void plot2d(cv::Mat& img, xt::xtensor<float, 2> xy)
{
	if (xy.shape()[0] == 2)
		plot2d(img, xt::view(xy, 0, xt::all()), xt::view(xy, 1, xt::all()));
	else if (xy.shape()[1] == 2)
		plot2d(img, xt::view(xy, xt::all(), 0), xt::view(xy, xt::all(), 1));
	else throw std::logic_error("plot2d(xy) expected 2xN or Nx2 matrix!");
}