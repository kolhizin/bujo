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
	public ref class ManagedDetector
	{
		unsigned timeLoad_, timeCompute_;
		bujo::detector::Detector* impl_;

		bujo::detector::Detector* impl();
	public:
		ManagedDetector();
		~ManagedDetector();
		!ManagedDetector();

		void LoadImage(Bitmap ^bmp);

		unsigned GetTimeLoad() { return timeLoad_; }
		unsigned GetTimeCompute() { return timeCompute_; }
		unsigned GetNumLines() { return impl()->numLines(); }
		unsigned GetNumWords() { return impl()->numWords(); }
		unsigned GetNumWords(unsigned lineId) { return impl()->numWords(lineId); }
		float GetAngle() { return impl()->textAngle(); }

		Bitmap ^GetAlignedImage();
		Bitmap ^GetWordImage(unsigned lineId, unsigned wordId, float yScale);
		Rectangle GetWordBBox(unsigned lineId, unsigned wordId, Size size, float yScale);

		void Destroy();
	};
}
