/** \file utils.h
This is a header file containing declaration of functions used to load the model data from files.
*/
#include <stdio.h>
#include <vector>
#include "mat.h"

#pragma once

struct Color32Native
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
};

///Defines a rectangle
struct MyRectangle
{
	double x, y, width, height;
};

///Given a pre allocated Mat it loads it from the specified file.
/**
@param filename name of the file to be read
@param mat pre allocated Mat object that will be filled
*/
template<typename T> 
void loadRaw(const char *filename, Mat<T> &mat)
{
	FILE *handle = fopen(filename, "rb");
	for (int i = 0; i < mat.height; i++)
		int res = fread(mat.mat[i], sizeof(T), mat.width, handle);

	fclose(handle);
}

///Given a Mat object it fills it with data from the specified file. The first bytes of the files have to contain the height and width of the Mat to be read.
/**
@param filename name of the file to be read
@param mat Mat object that will be filled
*/
template<typename T>
void loadRaw2(const char *filename, Mat<T> &mat)
{
	FILE *handle = fopen(filename, "rb");
	int h, w;
	fread(&h, sizeof(int), 1, handle);
	fread(&w, sizeof(int), 1, handle);

	mat = Mat<T>(h, w);

	for (int i = 0; i < mat.height; i++)
		int res = fread(mat.mat[i], sizeof(T), mat.width, handle);

	fclose(handle);
}
