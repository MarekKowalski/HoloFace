#pragma once


#include <iostream>
#include <iomanip>
#include<stdarg.h>
#include <vector>

#include "LapackWrap.h"

template <typename T>
class Mat
{
public:
	int height;
	int width;

	T **mat;
	T *data;

public:
	Mat();

	void setSize(int height, int width);

	T* getRow(int i) const;
	T* getData() const;

	Mat(int height, int width);

	Mat(int height, int width, T* data);

	Mat(const Mat<T> &other);
	Mat(const T *vec, int n_elements);

	~Mat();

	Mat<T>& operator=(const T &right);
	Mat<T>& operator=(const Mat<T> &right);
	Mat<T> operator-(const Mat<T> &right) const;
	Mat<T> operator-(const T &right) const;
	Mat<T> operator-=(const Mat<T> &right);
	Mat<T> operator-=(const T &right);
	
	Mat<T> operator+(const Mat<T> &right) const;
	Mat<T> operator+(const T &right) const;
	Mat<T> operator+=(const Mat<T> &right);
	Mat<T> operator+=(const T &right);
	Mat<T> operator*(const Mat<T> &right) const;
	void setRow(int row, std::initializer_list<T> list);
	void setColumn(int column, std::initializer_list<T> list);
	Mat<T> operator*(const T &right) const;
	float BLASdot(const Mat<float> &right) const;
	Mat<T> operator/(const T &right) const;
	Mat<T> operator/=(const T &right);

	Mat<T> diag();
	float trace();

	Mat<T> subsetRows(const Mat<int> &indexes);
    Mat<T> subsetRows(const int* indexes, int n_indexes);
	Mat<T> subsetCols(const Mat<int> &indexes);

    void copyFrom(const Mat<T> &source, int dest_row, int dest_col);
	void copyRow(const Mat<T> &source, int idestrow, int isourcerow);
	void copyColumn(const Mat<T> &source, int idestcol, int isourcecol);
		
	void subtractRow(const Mat<T> &right);
	void subtractColumn(const Mat<T> &right);
	void addRow(const Mat<T> &right);
	Mat<T> flipHorizontaly() const;


	T& operator()(int row, int col);
	T* operator[](int i) const { return getRow(i); }

	void swapRows(int i, int j);
	void swapColumns(int i, int j);
	double mean();

	Mat<T> tile(int n_h, int n_w) const;

	Mat<T> meanRow() const;
	Mat<T> minRow() const;
	Mat<T> maxRow() const;
	Mat<T> clip(T min, T max);

	double std();
	T norm();
	T norm(int start_col, int end_col, int start_row, int end_row);
	double flattenNorm(int start_col, int end_col, int start_row, int end_row, int n_el_in_row);
	void divideFlattened(int start_col, int end_col, int start_row, int end_row, int n_el_in_row, T val);
	
	Mat<float> inv();
	Mat<float> solve(Mat<float> &b);
	Mat<double> solve(Mat<double> &b);
	Mat<float> pinv();
	Mat<T> trans() const;
	Mat<T> flatten() const;
	Mat<T> flattenT() const;

	void svd(Mat<float> &U, Mat<float> &D, Mat<float> &V);


	T maximum() const;
	T minimum() const;
	void negateRow(int rowIndex);

	void print() const;
	Mat<T> subMat(int startRow, int endRow, int startCol, int endCol) const;
	Mat<T> vstack(Mat<T> other) const;
	Mat<T> hstack(Mat<T> other) const;
	
	int argmin() const;
	int argmax() const;
    
	T sum() const;
	Mat<float> sumRows();
	Mat<T> piecewisePow(double power) const;

	void copyFromUCharTable(unsigned char *src, int _width, int _height);
	void copyFromUCharTableWithFlip(unsigned char *src, int _width, int _height);

	void writeToOBJ(const char *filename);
		
	void writeToFile(const char* filename);
	bool readFromFile(const char* shortname);
	bool readVertexesFromObjFile(const char* filename);
	bool readFromPlyFile(const char* filename);


	static Mat<T> Mat<T>::eye(int size);
	static Mat<T> Mat<T>::ones(int height, int width);
};

template<typename U, typename T>
Mat<T> changeMatType(const Mat<U> &source)
{
	Mat<T> result(source.height, source.width);

	for (int i = 0; i < result.height; i++)
	{
		for (int j = 0; j < result.width; j++)
			result[i][j] = source[i][j];
	}

	return result;
}

#include "../../src/cTracker3/mat.cpp"