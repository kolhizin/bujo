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

void BuJoDetector::ManagedDetector::LoadImage()
{
	xt::xtensor<float, 2> tmp;
	impl()->loadImage(tmp);
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
