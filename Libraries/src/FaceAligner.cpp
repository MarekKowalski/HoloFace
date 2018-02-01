#include "FaceAligner.h"
#include "mat.h"
#include <MemoryBuffer.h>
#include <wrl.h>
#include <memory>

using namespace FaceProcessing;
using namespace Platform;

std::string WstringToString(const std::wstring& wstring)
{
	auto wideData = wstring.c_str();
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideData, -1, nullptr, 0, NULL, NULL);
	auto utf8 = std::make_unique<char[]>(bufferSize);
	WideCharToMultiByte(CP_UTF8, 0, wideData, -1, utf8.get(), bufferSize, NULL, NULL);

	return std::string(utf8.get());
}

FaceAligner::FaceAligner(Platform::String^ modelDir, int nIters)
{
	std::wstring modelDirWstr(modelDir->Data());
	std::string modelDirStr = WstringToString(modelDirWstr);

	alignment = FaceAlignment(modelDirStr, nIters);
}

Platform::Array<float>^ FaceAligner::LocalizeLandmarks(Windows::Graphics::Imaging::SoftwareBitmap^ image,
	int faceX, int faceY, int faceHeight, int faceWidth, int nIters)
{
	int imgHeight = image->PixelHeight;
	int imgWidth = image->PixelWidth;
	Array<float>^ landmarks = ref new Array<float>(2 * alignment.nPoints);

	Color32Native* nativeImage = SoftwareBitmapToArray(image);
	MyRectangle rect;
	rect.x = faceX;
	rect.y = faceY;
	rect.height = faceHeight;
	rect.width = faceWidth;

	Mat<float> matLandmarks = alignment.fitToRect(nativeImage, imgHeight, imgWidth, rect, nIters);

	memcpy(landmarks->Data, matLandmarks.data, alignment.nPoints * 2 * sizeof(float));

	return landmarks;
}

Platform::Array<float>^ FaceAligner::LocalizeLandmarks(Windows::Graphics::Imaging::SoftwareBitmap^ image,
	const Platform::Array<float>^ inputLandmarks, int nIters)
{
	int imgHeight = image->PixelHeight;
	int imgWidth = image->PixelWidth;

	Array<float>^ landmarks = ref new Array<float>(2 * alignment.nPoints);
	Color32Native* nativeImage = SoftwareBitmapToArray(image);

	Mat<float> matInputLandmarks(alignment.nPoints, 2, inputLandmarks->Data);
	Mat<float> matLandmarks = alignment.fitWithAffine(nativeImage, imgHeight, imgWidth, matInputLandmarks, nIters);

	memcpy(landmarks->Data, matLandmarks.data, alignment.nPoints * 2 * sizeof(float));

	return landmarks;
}

float FaceAligner::GetRanking(Windows::Graphics::Imaging::SoftwareBitmap^ image,
	const Platform::Array<float>^ inputLandmarks)
{
	int imgHeight = image->PixelHeight;
	int imgWidth = image->PixelWidth;

	Color32Native* nativeImage = SoftwareBitmapToArray(image);
	Mat<float> matInputLandmarks(alignment.nPoints, 2, inputLandmarks->Data);

	return alignment.getRanking(nativeImage, imgHeight, imgWidth, matInputLandmarks);
}

float FaceAligner::GetRanking()
{
	return alignment.getRanking();
}

Color32Native* FaceAligner::SoftwareBitmapToArray(Windows::Graphics::Imaging::SoftwareBitmap^ image)
{
	Windows::Graphics::Imaging::BitmapBuffer^ bitmapBuffer = image->LockBuffer(Windows::Graphics::Imaging::BitmapBufferAccessMode::Read);

	unsigned char* nativeData;
	unsigned int capacity;
	Microsoft::WRL::ComPtr<Windows::Foundation::IMemoryBufferByteAccess> bufferByteAccess;
	reinterpret_cast<IInspectable*>(bitmapBuffer->CreateReference())->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

	bufferByteAccess->GetBuffer(&nativeData, &capacity);

	Color32Native *nativeColorData = (Color32Native*)nativeData;
	return nativeColorData;
}