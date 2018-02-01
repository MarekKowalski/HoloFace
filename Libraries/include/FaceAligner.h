#pragma once
#include "FaceAlignment.h"
#include "utils.h"

namespace FaceProcessing
{
    public ref class FaceAligner sealed
    {
    public:
		FaceAligner(Platform::String^ modelDir, int nIters);

		Platform::Array<float>^ LocalizeLandmarks(Windows::Graphics::Imaging::SoftwareBitmap^ image,
			int faceX, int faceY, int faceHeight, int faceWidth, int nIters);
		Platform::Array<float>^ LocalizeLandmarks(Windows::Graphics::Imaging::SoftwareBitmap^ image,
			const Platform::Array<float>^ inputLandmarks, int nIters);

		float GetRanking(Windows::Graphics::Imaging::SoftwareBitmap^ image,
			const Platform::Array<float>^ inputLandmarks);
		float GetRanking();
	private:
		FaceAlignment alignment;

		Color32Native* SoftwareBitmapToArray(Windows::Graphics::Imaging::SoftwareBitmap^ image);
    };
}
