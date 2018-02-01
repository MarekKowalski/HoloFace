#include "common_processing.h"

Mat<unsigned char> affineTransform(Color32Native *img, int imgHeight, int imgWidth, Mat<float> &A, Mat<float> &t, int destH, int destW)
{
	Mat<unsigned char> img2(destH, destW);
	float a00 = A[0][0];
	float a10 = A[1][0];
	float a01 = A[0][1];
	float a11 = A[1][1];
	float t0 = t[0][0];
	float t1 = t[0][1];

	int widthMinusOne = imgWidth - 1;
	int heightMinusOne = imgHeight - 1;

#pragma omp parallel for
	for (int y = 0; y < destH; y++)
	{
		for (int x = 0; x < destW; x++)
		{
			int intX = (int)(x * a00 + y * a10 + t0 + 0.5f);
			int intY = (int)(x * a01 + y * a11 + t1 + 0.5f);

			if (intX > widthMinusOne || intX < 0 || intY > heightMinusOne || intY < 0)
				continue;

			Color32Native &color = img[intY * imgWidth + intX];
			img2[y][x] = (color.r + color.g + color.b) / 3;
		}
	}

	return img2;
}

Mat<unsigned char> straighten(Color32Native *img, int imgHeight, int imgWidth, Mat<float> &shape, Mat<float> &meanS, std::vector<int> *indexes)
{
	Mat<float> A, A2, t;
	Mat<float> destShape = meanS - meanS.minRow();
	Mat<float> offset = (meanS.maxRow() - meanS.minRow()) * 0.5;
	Mat<float> imSize = (meanS.maxRow() - meanS.minRow()) * 2;
	destShape = destShape + offset;
	
	Mat<float> org_shape = shape;

	if (indexes == NULL)
		align(destShape, shape, A, t);
	else
	{
		Mat<float> chosen_dest_points(indexes->size(), 2);
		Mat<float> chosen_shape_points(indexes->size(), 2);
		for (unsigned int i = 0; i < indexes->size(); i++)
			for (int j = 0; j < 2; j++)
			{
				int index = (*indexes)[i];
				chosen_dest_points[i][j] = destShape[index][j];
				chosen_shape_points[i][j] = shape[index][j];
			}
		align(chosen_dest_points, chosen_shape_points, A, t);
	}

	shape = shape * A + t;

	t = t * -1;
	A2 = A;


	A2[0][0] = A[1][1];
	A2[0][1] = -A[0][1];
	A2[1][0] = -A[1][0];
	A2[1][1] = A[0][0];

	A2 = A2 * (1 / (A[0][0] * A[1][1] - A[0][1] * A[1][0]));
	t = t * A2;

	Mat<unsigned char> outImg = affineTransform(img, imgHeight, imgWidth, A2, t, static_cast<int>(imSize[0][1]), static_cast<int>(imSize[0][0]));

	return outImg;
}

Mat<unsigned char> straighten(Color32Native *img, int imgHeight, int imgWidth, Mat<float> &shape, Mat<float> &canonicalShape, int outHeight, int outWidth)
{
	Mat<float> A, A2, t;

	align(canonicalShape, shape, A, t);

	shape = shape * A + t;

	t = t * -1;
	A2 = A;

	A2[0][0] = A[1][1];
	A2[0][1] = -A[0][1];
	A2[1][0] = -A[1][0];
	A2[1][1] = A[0][0];

	A2 = A2 * (1 / (A[0][0] * A[1][1] - A[0][1] * A[1][0]));
	t = t * A2;

	Mat<unsigned char> outImg = affineTransform(img, imgHeight, imgWidth, A2, t, outHeight, outWidth);

	return outImg;
}

Mat<float> align(Mat<float> &dest, Mat<float> &src, Mat<float> &A, Mat<float> &t)
{
	Mat<float> destMean = dest.meanRow();
	Mat<float> srcMean = src.meanRow();

	Mat<float> srcVec = (src - srcMean).flatten();
	Mat<float> destVec = (dest - destMean).flatten();


	float a = (srcVec * destVec.trans())[0][0] / pow(srcVec.norm(), 2);
	float b = 0;

	for (int i = 0; i < dest.height; i++)
	{
		b += srcVec[0][2 * i] * destVec[0][2 * i + 1] - srcVec[0][2 * i + 1] * destVec[0][2 * i];
	}
	b = b / pow(srcVec.norm(), 2);

	A = Mat<float>(2, 2);
	A[0][0] = a;
	A[0][1] = b;
	A[1][0] = -b;
	A[1][1] = a;

	srcMean = srcMean * A;
	t = destMean - srcMean;

	Mat<float> res = src * A + t;
	return res;
}

