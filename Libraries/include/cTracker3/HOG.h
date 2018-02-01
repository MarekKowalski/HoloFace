/** \file HOG.h
This is a header file for the HOG (Histogram of Oriented Gradients) descriptor extraction.
*/
#pragma once

#include "mat.h"

namespace hog
{
	///Extracts HOG descriptors with specified size at specified points (this version takes int points).
	/**
	@param img input image
	@param points (Nx2) matrix containing N points at which the descriptors will be extracted
	@param blockSize size of hog block (window) around each point measured in cellSizes, the entire window is a square with a side length of blockSize * cellSize pixels
	@param cellSize size of each hog cell in pixels
	@param interpolate 0 for no interpolation, 1 for interpolation inside each cell separately, 2 for interpolation both within and between cells
	@param normalization_mode 0 for normalization using L2 norm, 1 not used
	@return returns a matrix where Nth row contains the descriptor of the Nth point
	*/
	Mat<float> HOG(Mat<unsigned char> &img, Mat<int> &points, int blockSize, int cellSize);

	///Extracts HOG descriptors with specified size at specified points (this version takes float points).
	/**
	@param img input image
	@param points (Nx2) matrix containing N points at which the descriptors will be extracted
	@param blockSize size of hog block (window) around each point measured in cellSizes, the entire window is a square with a side length of blockSize * cellSize pixels
	@param cellSize size of each hog cell in pixels
	@param interpolate 0 for no interpolation, 1 for interpolation inside each cell separately, 2 for interpolation both within and between cells
	@param normalization_mode 0 for normalization using L2 norm, 1 not used
	@return returns a matrix where Nth row contains the descriptor of the Nth point
	*/
	Mat<float> HOG(Mat<unsigned char> &img, Mat<float> &points, int blockSize, int cellSize);

	void init();
}