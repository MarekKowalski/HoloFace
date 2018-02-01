/** \file common_processing.h
This is a header file containing declaration of common functions used for transforming images and shapes.
*/
#pragma once

#include "mat.h"
#include "utils.h"

///Finds an affine transform that best fits one shape to another.
/**
@param dest the destination shape, to which the source shape will be fitted
@param src the source shape
@param A after execution will contain the linear part of the affine transform
@param t after execution will contain the translation part of the affine transform
@return returns the aligned shape
*/
Mat<float> align(Mat<float> &dest, Mat<float> &src, Mat<float> &A, Mat<float> &t);

///Transforms an image using an affine transform.
/**
@param img input image
@param A linear part of the affine transform
@param t translation part of the affine transform
@param destH height of the output image
@param destW width of the output image
@return returns the transformed image
*/
Mat<unsigned char> affineTransform(Color32Native *img, int imgHeight, int imgWidth, Mat<float> &A, Mat<float> &t, int destH, int destW);

///Normalizes the input shape and image so that the face shape has a standard scale, rotation and location in the image.
/**
@param img input image
@pram shape input face shape
@param meanS average face shpe (it is a part of the pre trained model)
@param indexes currently not used
*/
Mat<unsigned char> straighten(Color32Native *img, int imgHeight, int imgWidth, Mat<float> &shape, Mat<float> &meanS, std::vector<int> *indexes = NULL);
Mat<unsigned char> straighten(Color32Native *img, int imgHeight, int imgWidth, Mat<float> &shape, Mat<float> &canonicalShape, int outHeight, int outWidth);
