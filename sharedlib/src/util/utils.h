#pragma once
#include <opencv2/opencv.hpp>
#include <xtensor/xarray.hpp>
#include <xtensor/xtensor.hpp>
#include <exception>
#include "quantiles.h"

namespace bujo
{
	namespace util
	{
		inline xt::xtensor<float, 2> cv2xt(const cv::Mat& src)
		{
			xt::xtensor<float, 2> res({ size_t(src.rows), size_t(src.cols) });
			switch (src.type())
			{
			case CV_32F:
				std::transform(src.begin<float>(), src.end<float>(), res.begin(),
					[](auto && r) {return r; });
				break;
			case CV_8U:
				std::transform(src.begin<unsigned char>(), src.end<unsigned char>(), res.begin(),
					[](auto && r) {return static_cast<float>(r) / 255.0f; });
				break;
			default: throw std::logic_error("Unexpected type in cv2xtf()!");
			}
			return res;
		}

		inline cv::Mat xt2cv(const xt::xarray<float> & src, int cvType)
		{
			int tensor_shape_local_[16];
			if (src.shape().size() > 16)
				throw std::runtime_error("Function xt2cv() received array with more than 16 dimensions!");
			for (int i = 0; i < src.shape().size(); i++)
				tensor_shape_local_[i] = static_cast<int>(src.shape()[i]);
			cv::Mat res;
			res.create(static_cast<int>(src.shape().size()), tensor_shape_local_, cvType);
			switch (cvType)
			{
			case CV_32F:
				std::transform(src.begin(), src.end(), res.begin<float>(),
					[](auto && r) {return r; });
				break;
			case CV_8U:
				std::transform(src.begin(), src.end(), res.begin<unsigned char>(),
					[](auto && r) {return static_cast<unsigned char>(std::roundf(r * 255.0f)); });
				break;
			default: throw std::logic_error("Unexpected type in xt2cv()!");;
			}
			return res;
		}
		
		template<int N>
		inline cv::Mat xt2cv(const xt::xtensor<float, N>& src, int cvType)
		{
			int tensor_shape_local_[N];
			for (int i = 0; i < src.shape().size(); i++)
				tensor_shape_local_[i] = static_cast<int>(src.shape()[i]);
			cv::Mat res;
			res.create(static_cast<int>(src.shape().size()), tensor_shape_local_, cvType);
			switch (cvType)
			{
			case CV_32F:
				std::transform(src.begin(), src.end(), res.begin<float>(),
					[](auto && r) {return r; });
				break;
			case CV_8U:
				std::transform(src.begin(), src.end(), res.begin<unsigned char>(),
					[](auto && r) {return static_cast<unsigned char>(std::roundf(r * 255.0f)); });
				break;
			default: throw std::logic_error("Unexpected type in xt2cv()!");;
			}
			return res;
		}
	}
}