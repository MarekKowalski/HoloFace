#pragma once
/** \file FaceAlignment.h
This is a header file for the FaceAlignment class, which contains the declations of all its members and methods.
*/
#include "mat.h"
#include "utils.h"
#include "tree.h"

///Main class of the FaceAlignment module, contains functionality that, provided initial face location, localizes a set of characteristic facial points in an image.
class FaceAlignment
{
public:
	Mat<float> meanS; ///< contains the average shape of a human face (once loaded with the model)
	int nIters; ///< predefined number of iterations of the loaded moded
	int nPoints; ///< number of points that the model localizes

	float rankingValue;

	FaceAlignment(); ///< An empty default constructor.

	///Loads a model consisting of nIters from a provided directory
	/**
	@param inDir a path to a directory contatining the model files
	@param nIters number of iterations of the model to be loaded
	*/
	FaceAlignment(string inDir, int nIters = 5);

	///Localizes the face points in an image, provided a rectangle containing the face.
	/**
	@param img an input image containing a face normalized for rotation, scaling and translation
	@param rect a rectangle containing the face
	@param iters the number of iterations that are to be performed, clipped if larger than nIters
	@return returns the fitted shape
	*/
	Mat<float> fitToRect(Color32Native *img, int imgHeight, int imgWidth, MyRectangle &rect, int iters);

	///Localizes the face points in an image, provided an initial set of face points.
	/**
	@param img an input image containing a face normalized for rotation, scaling and translation
	@param inShape an initial estimate of the faces location
	@param iters the number of iterations that are to be performed, clipped if larger than nIters
	@return returns the fitted shape
	*/
	Mat<float> fitWithAffine(Color32Native *img, int imgHeight, int imgWidth, Mat<float> &inShape, int iters);

	///Calculates a quality of fit estimation (ranking) of a given shape. The greater the ranking the lower the quality of fit.
	/**
	@param img input image
	@param inShape a previously fitted shape of a face in the input image
	@return returns the ranking value
	*/
	float getRanking(Color32Native *img, int imgHeight, int imgWidth, Mat<float> &inShape);
	float getRanking();
private:
	///Localizes the face points in a pre normalized image, provided an initial face location.
	/**
	@param img an input image containing a face normalized for rotation, scaling and translation
	@param inShape an initial estimate of the faces location
	@param iters the number of iterations that are to be performed, clipped if larger than nIters
	@return returns the fitted shape
	*/
	Mat<float> fit(Color32Native *img, int imgHeight, int imgWidth, Mat<float> &inShape, int iters);

	vector<Mat<float>> W; ///< Contains a regression matrix for each iteration of the tracking procedure.
	vector<Mat<float>> intercept; ///< For each iteration contains a constant vector that is added to the regression results (in most cases it is all 0).
	TreeFeature tree; ///< Regression trees for each tracking iteration and point.

	Mat<float> scales; ///< A scale to which the initial face area is resized (different for each iteration).

	Mat<float> rankingW; ///< A regression matrix used for estimating the quality of fit (ranking) of a given shape.
	Mat<int> rankingBlockCell; ///< Cell and block size for extracting the HOG features used for quality of fit estimation.
	float rankingIntercept; ///< A constant value added to the ranking regression result (in most cases 0).

	float lastRanking;
};