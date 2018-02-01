#include "ImageProcessing.h"
#include "mat.h"
#include <MemoryBuffer.h>
#include <wrl.h>
#include "common_processing.h"

using namespace Platform;
using namespace FaceProcessing;

void ImageProcessing::ColorToGrayscale(Windows::Graphics::Imaging::SoftwareBitmap^ image, int32 grayscaleData)
{
	int height = image->PixelHeight;
	int width = image->PixelWidth;
	unsigned char *nativeGrayscaleData = (unsigned char*)(void*)grayscaleData;

	Windows::Graphics::Imaging::BitmapBuffer^ bitmapBuffer = image->LockBuffer(Windows::Graphics::Imaging::BitmapBufferAccessMode::Read);

	unsigned char* nativeData;
	unsigned int capacity;
	Microsoft::WRL::ComPtr<Windows::Foundation::IMemoryBufferByteAccess> bufferByteAccess;
	reinterpret_cast<IInspectable*>(bitmapBuffer->CreateReference())->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

	bufferByteAccess->GetBuffer(&nativeData, &capacity);	
	Color32Native *nativeColorData = (Color32Native*)nativeData;
#pragma omp parallel for
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			Color32Native pixel = nativeColorData[j + i * width];
			nativeGrayscaleData[j + i * width] = (pixel.r + pixel.g + pixel.b) / 3;
		}
	}
}

Array<unsigned char>^ ImageProcessing::PrepareImageForBackend(Windows::Graphics::Imaging::SoftwareBitmap^ image,
	const Array<float>^ inputLandmarks, const Array<float>^ canonicalLandmarks, int outHeight, int outWidth)
{
	Windows::Graphics::Imaging::BitmapBuffer^ bitmapBuffer = image->LockBuffer(Windows::Graphics::Imaging::BitmapBufferAccessMode::Read);
	unsigned char* nativeData;
	unsigned int capacity;
	Microsoft::WRL::ComPtr<Windows::Foundation::IMemoryBufferByteAccess> bufferByteAccess;
	reinterpret_cast<IInspectable*>(bitmapBuffer->CreateReference())->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

	bufferByteAccess->GetBuffer(&nativeData, &capacity);
	Color32Native *nativeColorData = (Color32Native*)nativeData;

	Mat<float> matInputLandmarks(inputLandmarks->Length / 2, 2, inputLandmarks->Data);
	Mat<float> matCanonicalLandmarks(canonicalLandmarks->Length / 2, 2, canonicalLandmarks->Data);

	Mat<unsigned char> outImg = straighten(nativeColorData, image->PixelHeight, image->PixelWidth, matInputLandmarks, matCanonicalLandmarks, outHeight, outWidth);

	Array<unsigned char>^ output = ref new Array<unsigned char>(outHeight * outWidth);
	memcpy(output->Data, outImg.data, outHeight * outWidth);

	return output;
}