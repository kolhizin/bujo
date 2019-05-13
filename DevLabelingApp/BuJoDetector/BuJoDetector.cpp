#include <src/detector.h>
#include "BuJoDetector.h"
#include <xtensor/xview.hpp>
#using <System.Windows.Forms.Dll>
using namespace System::Windows::Forms;

bujo::detector::Detector* BuJoDetector::ManagedDetector::impl()
{
	if (!impl_)
		throw gcnew System::NullReferenceException("ManagedDetector already destroyed!");
	return impl_;
}

BuJoDetector::ManagedDetector::ManagedDetector() : impl_(new bujo::detector::Detector)
{
}

BuJoDetector::ManagedDetector::~ManagedDetector()
{
	Destroy();
}

BuJoDetector::ManagedDetector::!ManagedDetector()
{
	Destroy();
}

void BuJoDetector::ManagedDetector::LoadImage(Bitmap^ bmp, float sizeFactor)
{

	auto sw = System::Diagnostics::Stopwatch::StartNew();
	size_t w = bmp->Width, h = bmp->Height;

	xt::xtensor<float, 2> tmp({ h, w });
	auto bmpData = bmp->LockBits(Rectangle(0, 0, w, h),
		Imaging::ImageLockMode::ReadOnly, bmp->PixelFormat);

	int numChannels = 1;
	if (bmpData->PixelFormat == Imaging::PixelFormat::Format24bppRgb)
		numChannels = 3;
	else if (bmpData->PixelFormat == Imaging::PixelFormat::Format32bppArgb)
		numChannels = 4;
	else gcnew System::ArgumentException("Unexpected pixel format!");
	int stride = bmpData->Stride;
	unsigned char* ptr = static_cast<unsigned char*>(static_cast<void *>(bmpData->Scan0));

	for(int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
		{
			unsigned char* p = &ptr[i * stride + j * numChannels];
			float r = 0.0f;
			if (bmpData->PixelFormat == Imaging::PixelFormat::Format24bppRgb)
				r = (p[0] + p[1] + p[2]) / 3.0f;
			else if (bmpData->PixelFormat == Imaging::PixelFormat::Format32bppArgb)
				r = (p[1] + p[2] + p[3]) / 3.0f;
			tmp.at(i, j) = r / 255.0f;
		}
	bmp->UnlockBits(bmpData);
	timeLoad_ = sw->ElapsedMilliseconds;
	
	impl()->loadImage(tmp, sizeFactor);
	impl()->updateRegionAuto(1.2f, 100, 10.0f, 0.0f, 0.05f);
	//impl()->selectSupportCurvesAuto(6, 25);
	//impl()->detectWords(25, 5);

	timeCompute_ = sw->ElapsedMilliseconds - timeLoad_;
}

Bitmap ^xt2bitmap(const xt::xtensor<float, 2>& src)
{
	unsigned w = src.shape()[1], h = src.shape()[0];
	Imaging::PixelFormat pixelFormat = Imaging::PixelFormat::Format24bppRgb;
	Bitmap^ res = gcnew Bitmap(w, h, pixelFormat);
	auto bmpData = res->LockBits(Rectangle(0, 0, w, h), Imaging::ImageLockMode::ReadOnly, pixelFormat);
	
	int numChannels = 3;
	int stride = bmpData->Stride;
	unsigned char* ptr = static_cast<unsigned char*>(static_cast<void*>(bmpData->Scan0));

	for (int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
		{
			unsigned char* p = &ptr[i * stride + j * numChannels];
			unsigned char r = static_cast<unsigned char>(src.at(i, j) * 255.0f);
			p[0] = r;
			p[1] = r;
			p[2] = r;
		}
	res->UnlockBits(bmpData);
	return res;
}

Bitmap ^BuJoDetector::ManagedDetector::GetAlignedImage()
{
	return xt2bitmap(impl()->alignedOriginalImage());
}

Bitmap^ BuJoDetector::ManagedDetector::GetMainImage()
{
	auto tmp = impl()->mainImage();
	return xt2bitmap(tmp / xt::amax(tmp)[0]);
}

Bitmap^ BuJoDetector::ManagedDetector::GetTextImage()
{
	return xt2bitmap(impl()->textImage());
}

Bitmap^ BuJoDetector::ManagedDetector::GetFilteredImage()
{
	auto tmp = impl()->filteredImage();
	return xt2bitmap(tmp / xt::amax(tmp)[0]);
}

Bitmap ^BuJoDetector::ManagedDetector::GetWordImage(unsigned lineId, unsigned wordId, float yScale)
{
	return xt2bitmap(impl()->extractWord(lineId, wordId, yScale));
}

Rectangle BuJoDetector::ManagedDetector::GetWordBBox(unsigned lineId, unsigned wordId, Size size, float yScale)
{
	xt::xtensor<float, 2> locs({ 4, 2 });
	locs.at(0, 0) = 0.0f;	locs.at(0, 1) = -yScale;
	locs.at(1, 0) = 1.0f;	locs.at(1, 1) = -yScale;
	locs.at(2, 0) = 1.0f;	locs.at(2, 1) = yScale;
	locs.at(3, 0) = 0.0f;	locs.at(3, 1) = yScale;
	auto globs = impl()->wordCoordinates(lineId, wordId, locs);

	auto xv = xt::view(globs, xt::all(), 0);
	auto yv = xt::view(globs, xt::all(), 1);

	float xmin = xt::amin(xv)[0], xmax = xt::amax(xv)[0];
	float ymin = xt::amin(yv)[0], ymax = xt::amax(yv)[0];

	float fx0 = std::floorf(xmin * size.Width);
	float fy0 = std::floorf(ymin * size.Height);
	float fw = std::ceilf((xmax - xmin) * size.Width);
	float fh = std::ceilf((ymax - ymin) * size.Height);

	return Rectangle(fx0, fy0, fw, fh);
}

void BuJoDetector::ManagedDetector::Destroy()
{
	if (!impl_)
		return;
	delete impl_;
	impl_ = nullptr;
}
