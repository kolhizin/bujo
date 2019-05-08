#pragma once

using namespace System;

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

		void LoadImage();

		unsigned GetTextLineDelta();

		void Destroy();
	};
}
