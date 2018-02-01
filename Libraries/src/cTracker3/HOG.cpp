#include "hog.h"
#include <assert.h>
#include <thread>

#define PI 3.141592f
#define M_PI_4 0.785398f
#define NTHREADS 4
#define CELL_SIZE 4
#define BLOCK_SIZE 4

namespace hog
{
	float fastAtan2(float y, float x)
	{
		static float tab[1024 * 1024] = { 0 };

		if (!tab[0])
		{
			for (int i = 0; i < 1024; i++)
			{
				for (int j = 0; j < 1024; j++)
				{
					int x = i - 512;
					int y = j - 512;
					tab[j + 1024 * i] = atan2f((float)y, (float)x);
				}
			}
		}

		return tab[(int)y + 512 + 1024 * ((int)x + 512)];
	}
	
	void addToBin(float *cbins, int bin, float w, float dInt, float dIntL, float dIntR, int nbins)
	{
		if (bin > 0)
			cbins[bin - 1] += dIntL * w;
		else
			cbins[nbins - 1] += dIntL * w;

		if (bin < nbins - 1)
			cbins[bin + 1] += dIntR * w;
		else
			cbins[0] += dIntR * w;
		cbins[bin] += dInt * w;
	}

	void init()
	{
		fastAtan2(1, 1);
	}


	void initializeMultXY(float multXY[CELL_SIZE * CELL_SIZE][4])
	{
		int startY = 0;
		int startX = 0;
		int cellSize = CELL_SIZE;
		int cellCenterX = (int)(startX + (cellSize - 1) / 2.0f);
		int cellCenterY = (int)(startY + (cellSize - 1) / 2.0f);

		int idx = 0;
		for (int y = startY; y < startY + cellSize; y++)
		{
			for (int x = startX; x < startX + cellSize; x++)
			{
				float x1, y1;
				if (x <= cellCenterX && y <= cellCenterY)
				{
					x1 = (float)(cellCenterX - cellSize);
					y1 = (float)(cellCenterY - cellSize);
				}
				else if (x <= cellCenterX && y > cellCenterY)
				{
					x1 = (float)(cellCenterX - cellSize);
					y1 = (float)(cellCenterY);
				}
				else if (x > cellCenterX && y <= cellCenterY)
				{
					x1 = (float)(cellCenterX);
					y1 = (float)(cellCenterY - cellSize);
				}
				else if (x > cellCenterX && y > cellCenterY)
				{
					x1 = (float)(cellCenterX);
					y1 = (float)(cellCenterY);
				}

				float multX = 1 - (x - x1) / cellSize;
				float multY = 1 - (y - y1) / cellSize;
				multXY[idx][0] = multX * multY;

				multX = 1 - (x - x1) / cellSize;
				multY = (y - y1) / cellSize;
				multXY[idx][1] = multX * multY;

				multX = (x - x1) / cellSize;
				multY = 1 - (y - y1) / cellSize;
				multXY[idx][2] = multX * multY;

				multX = (x - x1) / cellSize;
				multY = (y - y1) / cellSize;
				multXY[idx][3] = multX * multY;

				idx++;
			}
		}
	}

	int checkBlockBoundary(int blockX, int blockY, int offsetX, int offsetY, int blockSize, int nbins)
	{
		if (blockX + offsetX >= blockSize || blockX + offsetX < 0)
			return -1;

		if (blockY + offsetY >= blockSize || blockY + offsetY < 0)
			return -1;

		return nbins * (blockX + offsetX + (blockY + offsetY) * blockSize);
	}

	void initializeBlockIdxs(int blockIdxs[CELL_SIZE * BLOCK_SIZE][CELL_SIZE * BLOCK_SIZE][4], int nbins)
	{
		int cellSize = CELL_SIZE;
		int blockSize = BLOCK_SIZE;



		for (int blockY = 0; blockY < blockSize; blockY++)
		{
			for (int blockX = 0; blockX < blockSize; blockX++)
			{
				int offset = nbins * (blockX + blockY * blockSize);

				int startX = cellSize * blockX;
				int startY = cellSize * blockY;

				int cellCenterX = (int)(startX + (cellSize - 1) / 2.0f);
				int cellCenterY = (int)(startY + (cellSize - 1) / 2.0f);

				for (int y = startY; y < startY + cellSize; y++)
				{
					for (int x = startX; x < startX + cellSize; x++)
					{
						if (x <= cellCenterX && y <= cellCenterY)
						{
							blockIdxs[x][y][0] = checkBlockBoundary(blockX, blockY, -1, -1, blockSize, nbins);// offset + nbins * (-blockSize - 1);
							blockIdxs[x][y][1] = checkBlockBoundary(blockX, blockY, -1, 0, blockSize, nbins); //offset + nbins * (-1);
							blockIdxs[x][y][2] = checkBlockBoundary(blockX, blockY, 0, -1, blockSize, nbins); //offset + nbins * (-blockSize);
							blockIdxs[x][y][3] = checkBlockBoundary(blockX, blockY, 0, 0, blockSize, nbins); //offset + nbins * (0);
						}
						else if (x <= cellCenterX && y > cellCenterY)
						{
							blockIdxs[x][y][0] = checkBlockBoundary(blockX, blockY, -1, 0, blockSize, nbins);//offset + nbins * (-1);
							blockIdxs[x][y][1] = checkBlockBoundary(blockX, blockY, -1, 1, blockSize, nbins);//offset + nbins * (blockSize - 1);
							blockIdxs[x][y][2] = checkBlockBoundary(blockX, blockY, 0, 0, blockSize, nbins);//offset + nbins * (0);
							blockIdxs[x][y][3] = checkBlockBoundary(blockX, blockY, 0, 1, blockSize, nbins);//offset + nbins * (blockSize);
						}
						else if (x > cellCenterX && y <= cellCenterY)
						{
							blockIdxs[x][y][0] = checkBlockBoundary(blockX, blockY, 0, -1, blockSize, nbins);//offset + nbins * (-blockSize);
							blockIdxs[x][y][1] = checkBlockBoundary(blockX, blockY, 0, 0, blockSize, nbins);//offset + nbins * (0);
							blockIdxs[x][y][2] = checkBlockBoundary(blockX, blockY, 1, -1, blockSize, nbins);//offset + nbins * (-blockSize + 1);
							blockIdxs[x][y][3] = checkBlockBoundary(blockX, blockY, 1, 0, blockSize, nbins);//offset + nbins * (1);
						}
						else if (x > cellCenterX && y > cellCenterY)
						{
							blockIdxs[x][y][0] = checkBlockBoundary(blockX, blockY, 0, 0, blockSize, nbins);//offset + nbins * (0);
							blockIdxs[x][y][1] = checkBlockBoundary(blockX, blockY, 0, 1, blockSize, nbins);//offset + nbins * (blockSize);
							blockIdxs[x][y][2] = checkBlockBoundary(blockX, blockY, 1, 0, blockSize, nbins);//offset + nbins * (1);
							blockIdxs[x][y][3] = checkBlockBoundary(blockX, blockY, 1, 1, blockSize, nbins);//offset + nbins * (blockSize + 1);
						}
					}
				}
			}
		}
	}

	void histogram(float *cbins, float *cdata, float *cweights, int nbins, int min, int max, int startX, int startY, int cellSize, int blockSize)
	{
		static float multXY[CELL_SIZE * CELL_SIZE][4] = { 0 };
		static int blockIdxs[CELL_SIZE * BLOCK_SIZE][CELL_SIZE * BLOCK_SIZE][4] = { 0 };
		static bool initialized = false;
		if (!initialized)
		{
			initializeMultXY(multXY);
			initializeBlockIdxs(blockIdxs, nbins);
			initialized = true;
		}				

		int width = CELL_SIZE * BLOCK_SIZE;
		float fmax = max * 1.00001f;
		float bin_width = (fmax - min) / (float)nbins;
		float inv_bin_width = 1 / bin_width;

		int bin;
		float dIntL, dIntR, fbin, dtmp;
		float a, w, dInt;

		int idx = 0;
		for (int y = startY; y < startY + cellSize; y++)
		{
			for (int x = startX; x < startX + cellSize; x++)
			{
				int i = x + width * y;
				a = cdata[i];
				w = cweights[i];

				fbin = (a - min) * inv_bin_width;
				bin = static_cast<int>(fbin);

				dInt = 1.0;
				dIntL = 0;
				dIntR = 0;
				dtmp = (float)bin + 0.5f - fbin;

				if (dtmp > 0)
				{
					dIntL = dtmp;
					dInt = dInt - dIntL;
				}

				dtmp = fbin - ((float)bin + 0.5f);

				if (dtmp > 0)
				{
					dIntR = dtmp;
					dInt = dInt - dIntR;
				}

				int *tempBlockIdxs = blockIdxs[x][y];
				float *tempMultyXY = multXY[idx];

				if (tempBlockIdxs[0] != -1)
					addToBin(cbins + tempBlockIdxs[0], bin, tempMultyXY[0] * w,
						dInt, dIntL,
						dIntR, nbins);

				if (tempBlockIdxs[1] != -1)
					addToBin(cbins + tempBlockIdxs[1], bin, tempMultyXY[1] * w,
						dInt, dIntL,
						dIntR, nbins);

				if (tempBlockIdxs[2] != -1)
					addToBin(cbins + tempBlockIdxs[2], bin, tempMultyXY[2] * w,
						dInt, dIntL,
						dIntR, nbins);

				if (tempBlockIdxs[3] != -1)
					addToBin(cbins + tempBlockIdxs[3], bin, tempMultyXY[3] * w,
						dInt, dIntL,
						dIntR, nbins);

				idx++;
			}
		}
	}

	void gradientsAroundPoint(unsigned char *img, float *gradsX, float *gradsY, int pointX, int pointY, int totalBlockSize, int w, int h)
	{
		int offset = static_cast<int>((totalBlockSize / 2.0f) + 0.5f);
		int startX = pointX - offset;
		int startY = pointY - offset;
		int endX = pointX + offset;
		int endY = pointY + offset;

		for (int x = startX, x2 = 0; x < endX; x++, x2++)
		{
			if (x - 1 < 0 || x + 1 >= w)
				continue;
			for (int y = startY, y2 = 0; y < endY; y++, y2++)
			{
				if (y - 1 < 0 || y + 1 >= h)
					continue;

				gradsX[x2 + totalBlockSize * y2] = (img[x + 1 + w * y] - img[x - 1 + w * y]) * 0.5f;
				gradsY[x2 + totalBlockSize * y2] = (img[x + w * (y + 1)] - img[x + w * (y - 1)]) * 0.5f;
			}
		}
	}

	void processGrads(Mat<float> &gradsX, Mat<float> &gradsY, Mat<float> &angles, Mat<float> &magnitudes)
	{
		int height = angles.height, width = angles.width;

		float val, tempGradX, tempGradY;
		float scale = 180.0f / PI;


		for (int i = 0; i < height * width; i++)
		{
			tempGradY = gradsY.data[i];
			tempGradX = gradsX.data[i];

			val = fastAtan2(tempGradY, tempGradX) * scale;
			if (val < 0)
				val += 180;
			else if (val > 180)
				val = 180;
			angles.data[i] = val;
			magnitudes.data[i] = sqrt(tempGradY * tempGradY + tempGradX * tempGradX);
		}
	}

	Mat<float> HOG(Mat<unsigned char> &img, Mat<float> &points, int blockSize, int cellSize)
	{
		Mat<int> intPoints(points.height, points.width);

		for (int i = 0; i < intPoints.height; i++)
		{
			for (int j = 0; j < intPoints.width; j++)
			{
				intPoints[i][j] = (int)(points[i][j] + 0.5f);
			}
		}

		return HOG(img, intPoints, blockSize, cellSize);
	}


	Mat<float> HOG(Mat<unsigned char> &img, Mat<int> &points, int blockSize, int cellSize)
	{
		int nbins = 9;
		Mat<float> res(points.height, blockSize * blockSize * nbins);

		Mat<float> gradsX(blockSize * cellSize, blockSize * cellSize), gradsY(blockSize * cellSize, blockSize * cellSize);
		Mat<float> angles(blockSize * cellSize, blockSize * cellSize);
		Mat<float> magnitudes(blockSize * cellSize, blockSize * cellSize);

		Mat<float> temp(blockSize * blockSize, nbins);

#pragma omp parallel for firstprivate(gradsX, gradsY, angles, magnitudes, temp)
		for (int i = 0; i < points.height; i++)
		{
			temp = 0;

			gradientsAroundPoint(img.data, gradsX.data, gradsY.data, points[i][0], points[i][1], blockSize * cellSize, img.width, img.height);
			processGrads(gradsX, gradsY, angles, magnitudes);

			for (int y = 0; y < blockSize; y++)
			{
				for (int x = 0; x < blockSize; x++)
				{
					histogram(temp.data, angles.data, magnitudes.data, nbins, 0, 180, x * cellSize, y * cellSize, cellSize, blockSize);
				}
			}						
	
			temp = temp / (temp.norm() + 1e-8f);

			memcpy(res.mat[i], temp.data, sizeof(float) * blockSize * blockSize * nbins);
		}

		return res;
	}
}