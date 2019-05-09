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
		bujo::detector::Detector* impl_;

		bujo::detector::Detector* impl();
	public:
		ManagedDetector();
		~ManagedDetector();
		!ManagedDetector();

		void LoadImage(Bitmap ^bmp);

		unsigned GetTextLineDelta();

		void Destroy();
	};
}
