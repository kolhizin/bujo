#pragma once
#using <System.Drawing.dll>

using namespace System;
using namespace System::Drawing;

namespace bujo{
	namespace detector {
		class Detector;
	}
}

namespace BuJoDetector {
	public ref class DetectorSettings
	{
	public:
		float maximumTextRotationAngle = 1.57f;
	};
	public ref class ManagedDetector
	{
		DetectorSettings ^settings_;
		unsigned timeLoad_, timeCompute_;
		bujo::detector::Detector* impl_;

		bujo::detector::Detector* impl();
	public:
		ManagedDetector();
		~ManagedDetector();
		!ManagedDetector();

		void LoadImage(Bitmap ^bmp, float sizeFactor);
		void RunDetection();

		DetectorSettings^ GetSettings() { return settings_; }
		void SetSettings(DetectorSettings^ settings) { settings_ = settings; }

		unsigned GetTimeLoad() { return timeLoad_; }
		unsigned GetTimeCompute() { return timeCompute_; }
		unsigned GetNumSupportLines() { return impl()->numSupportLines(); }
		unsigned GetNumLines() { return impl()->numLines(); }
		unsigned GetNumWords() { return impl()->numWords(); }
		unsigned GetNumWords(unsigned lineId) { return impl()->numWords(lineId); }
		float GetAngle() { return impl()->textAngle(); }

		array<PointF^>^ GetSupportLine(unsigned idx, unsigned numPoints);
		array<PointF^>^ GetLine(unsigned idx, unsigned numPoints);
		array<PointF^>^ GetWord(unsigned lineId, unsigned wordId, unsigned numPoints);
		float GetWordNegOffset(unsigned lineId, unsigned wordId);
		float GetWordPosOffset(unsigned lineId, unsigned wordId);

		Bitmap ^GetAlignedImage();
		Bitmap^ GetMainImage();
		Bitmap^ GetTextImage();
		Bitmap^ GetFilteredImage();
		Bitmap ^GetWordImage(unsigned lineId, unsigned wordId, float yScale);
		Rectangle GetWordBBox(unsigned lineId, unsigned wordId, Size size, float yScale);

		void Destroy();
	};
}
