#include "FaceAlignment.h"
#include "hog.h"
#include "common_processing.h"
#include <string>
#include "hog.h"


FaceAlignment::FaceAlignment() {}

FaceAlignment::FaceAlignment(string inDir, int nIters)
{
	int nBins = 9;
	this->nIters = nIters;

	hog::init();

	Mat<int> settings(1, 3);
	loadRaw((inDir + "settings.bin").c_str(), settings);
	nPoints = settings.data[0];
	int blockSize = settings.data[1];
	int cellSize = settings.data[2];

	tree = TreeFeature(inDir, blockSize, cellSize, nIters);

	meanS = Mat<float>(nPoints, 2);
	loadRaw((inDir + "meanS.bin").c_str(), meanS);

	//dodac czytanie dlugosci W
	for (int i = 0; i < nIters; i++)
	{
		W.push_back(Mat<float>());
		intercept.push_back(Mat<float>(2 * nPoints, 1));

		loadRaw2<float>((inDir + "W" + std::to_string(i) + ".bin").c_str(), W[i]);
		loadRaw((inDir + "intercept" + std::to_string(i) + ".bin").c_str(), intercept[i]);

		W[i] = W[i].trans();
	}

	scales = Mat<float>(1, nIters);
	loadRaw((inDir + "scales.bin").c_str(), scales);

	rankingBlockCell = Mat<int>(1, 2);
	loadRaw((inDir + "rankingBlockCell.bin").c_str(), rankingBlockCell);

	rankingW = Mat<float>(1, static_cast<int>(nBins * nPoints * pow((double)rankingBlockCell[0][0], 2)));
	loadRaw((inDir + "rankingW.bin").c_str(), rankingW);

	Mat<float> rankingIntercept(1, 1);
	loadRaw((inDir + "rankingIntercept.bin").c_str(), rankingIntercept);
	this->rankingIntercept = rankingIntercept[0][0];
}

Mat<float> FaceAlignment::fitToRect(Color32Native *img, int imgHeight, int imgWidth, MyRectangle &rect, int iters)
{
	float scale = static_cast<float>(0.9f * rect.width / (meanS.maxRow()[0][0] - meanS.minRow()[0][0]));
	Mat<float> offset(1, 2);
	offset.mat[0][0] = static_cast<float>(rect.x + 0.5 * rect.width);
	offset.mat[0][1] = static_cast<float>(rect.y + 0.5 * rect.height + scale * 10);
	Mat<float> shape = meanS * scale + offset;

	shape = fitWithAffine(img, imgHeight, imgWidth, shape, iters);

	return shape;
}

Mat<float> FaceAlignment::fitWithAffine(Color32Native *img, int imgHeight, int imgWidth, Mat<float> &inShape, int iters)
{
	Mat<float> shape;
	Mat<float> A, t;

	inShape = align(inShape, meanS, A, t);
	shape = fit(img, imgHeight, imgWidth, inShape, iters);


	return shape;
}

template <typename T>
Mat<T> multiply(const Mat<T> &left, const Mat<T> right)
{
	Mat<T> res(left.height, right.width);
	res = 0;
	int rh = res.height;
	int rw = res.width;

#pragma omp parallel for
	for (int i = 0; i < rh; i++)
	{
		T *ptr_this_row;
		T *ptr_res;
		T *ptr_right;

		ptr_res = res.getRow(i);
		ptr_this_row = left.getRow(i);
		ptr_right = right.data;

		for (int k = 0; k < left.width; k++)
		{
			*ptr_res += *ptr_this_row * *ptr_right;
			ptr_this_row++;
			ptr_right++;
		}
	}

	return res;
}

Mat<float> FaceAlignment::fit(Color32Native *img, int imgHeight, int imgWidth, Mat<float> &inShape, int iters)
{
	Mat<float> A, t;
	Mat<float> destShape = meanS - meanS.minRow();
	Mat<float> offset = (meanS.maxRow() - meanS.minRow()) * 0.5;
	Mat<float> imSize = (meanS.maxRow() - meanS.minRow()) * 2;
	destShape = destShape + offset;

	Mat<float> shape(inShape);
	Mat<unsigned char> img2;

	if (iters > nIters)
		iters = nIters;

	for (int iter = 0; iter < iters; iter++)
	{
		Mat<float> scaledDest = destShape * scales[0][iter];
		Mat<float> scaledImSize = imSize *  scales[0][iter];
		shape = align(scaledDest, shape, A, t);

		Mat<float> A2 = A;
		A2[0][0] = A[1][1];
		A2[0][1] = -A[0][1];
		A2[1][0] = -A[1][0];
		A2[1][1] = A[0][0];
		A2 = A2 * (1 / (A[0][0] * A[1][1] - A[0][1] * A[1][0]));
		A = A2;
		t = t * A * -1;

		img2 = affineTransform(img, imgHeight, imgWidth, A, t, static_cast<int>(scaledImSize[0][1]), static_cast<int>(scaledImSize[0][0]));

		Mat<int> features(tree.nOutputs[iter], 1);
		Mat<float> baseFeature;
		Mat<float> temp(W[iter].width, 1);
		temp = 0;
	
		
		tree.getBinaryFeature(img2, shape, features, baseFeature, iter);

		for (int i = 0; i < features.height; i++)
		{
			for (int j = 0; j < temp.height; j++)
			{
				temp[j][0] += W[iter][features[i][0]][j];
			}
		}

		for (int i = 0; i < nPoints; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				shape[i][j] = shape[i][j] + scales[0][iter] * temp[j + 2 * i][0];
			}
		}

		shape = shape * A + t;

		if (iter == iters - 1)
		{
			lastRanking = multiply(rankingW, baseFeature)[0][0] + rankingIntercept;
		}
	}

	return shape;
}

float FaceAlignment::getRanking(Color32Native *img, int imgHeight, int imgWidth, Mat<float> &inShape)
{
	float res;
	Mat<float> S = inShape;
	Mat<unsigned char> img2 = straighten(img, imgHeight, imgWidth, S, meanS);

	Mat<float> features = hog::HOG(img2, S, rankingBlockCell[0][0], rankingBlockCell[0][1]).flattenT();
	res = multiply(rankingW, features)[0][0] + rankingIntercept;

	return res;
}

float FaceAlignment::getRanking()
{
	return lastRanking;
}