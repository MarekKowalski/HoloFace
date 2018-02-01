#pragma once
#include "utils.h"

namespace FaceProcessing
{
	public ref class ImageProcessing sealed
	{
	public:
		ImageProcessing() {}
		static void ColorToGrayscale(Windows::Graphics::Imaging::SoftwareBitmap^ image, int32 grayscaleData);
		static Platform::Array<unsigned char>^ PrepareImageForBackend(Windows::Graphics::Imaging::SoftwareBitmap^ image,
			const Platform::Array<float>^ inputLandmarks, const Platform::Array<float>^ canonicalLandmarks, int outHeight, int outWidth);
	};
}
