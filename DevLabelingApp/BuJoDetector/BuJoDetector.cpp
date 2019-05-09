#include <src/detector.h>
#include "BuJoDetector.h"

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

void BuJoDetector::ManagedDetector::LoadImage(Bitmap^ bmp)
{
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

	impl()->loadImage(tmp, 0.1f);
	impl()->updateRegionAuto(1.2f, 100, 10.0f, 0.0f, 0.05f);
	impl()->selectSupportCurvesAuto(6, 25);
	impl()->detectWords(25, 5);
}

unsigned BuJoDetector::ManagedDetector::GetTextLineDelta()
{
	return impl()->textDelta();
}

void BuJoDetector::ManagedDetector::Destroy()
{
	if (!impl_)
		return;
	delete impl_;
	impl_ = nullptr;
}
