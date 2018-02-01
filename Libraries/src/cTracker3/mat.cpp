#include "mat.h"
#include "LapackWrap.h"
#include <algorithm>

using namespace std;

template <typename T>
Mat<T>::Mat()
{
	height = 1;
	width = 1;

	data = new T[height * width]();
	mat = new T*[height];

	for (int i = 0; i < height; i++)
		mat[i] = &data[width * i];
}

template<typename T>
Mat<T>::Mat(const T *vec, int n_elements)
{
	data = NULL;
	mat = NULL;
	setSize(n_elements, 1);
	for (int i = 0; i < n_elements; i++)
		data[i] = vec[i];
}

template<typename T>
void Mat<T>::negateRow(int rowIndex)
{
	T *row = mat[rowIndex];
	for (int i = 0; i < width; i++)
		row[i] *= -1.0;
}

template<typename T>
Mat<T> Mat<T>::flipHorizontaly() const
{
	Mat<T> res(height, width);

	for (int i = 0, i2 = width - 1; i < width; i++, i2--)
	{
		for (int j = 0; j < height; j++)
		{
			res[j][i] = mat[j][i2];
		}
	}

	return res;
}

template <typename T>
Mat<T> Mat<T>::tile(int n_h, int n_w) const
{
	Mat<T> tiled(height * n_h, width * n_w);

	for (int i = 0; i < n_h; i++)
	for (int j = 0; j < n_w; j++)
	for (int k = 0; k < width; k++)
	for (int l = 0; l < height; l++)
		tiled[i*height + l][j*width + k] = mat[l][k];


	return tiled;

}

template <typename T>
void Mat<T>::copyColumn(const Mat<T> &source, int idestcol, int isourcecol)
{
	for (int i = 0; i < height; i++)
		mat[i][idestcol] = source[i][isourcecol];
}


template <typename T>
void Mat<T>::copyRow(const Mat<T> &source, int idestrow, int isourcerow)
{
	T* src = source[isourcerow];
	T* dst = mat[idestrow];

	memcpy(dst, src, sizeof(T)* width);

}


template <typename T>
void Mat<T>::writeToFile(const char* shortname)
{
	string dim_name = string(shortname) + ".dim";
	string data_name = string(shortname) + ".txt";
	FILE *f;

	f = fopen(dim_name.c_str(), "wt");

	fprintf(f, "%d %d\n", height, width);
	fclose(f);

	f = fopen(data_name.c_str(), "wt");
	for (int i = 0; i<height; i++)
	{
		for (int j = 0; j<width; j++)
		{
			fprintf(f, "%f ", static_cast<float>(getRow(i)[j]));
		}
		fprintf(f, "\n");
	}

	fclose(f);
}

template <typename T>
void Mat<T>::setSize(int height, int width)
{
	if (data != NULL)
		delete[]data;
	if (mat != NULL)
		delete[]mat;

	this->width = width;
	this->height = height;

	data = new T[height * width]();
	mat = new T*[height];

	T *data_p = data;
	for (int i = 0; i < height; i++)
	{
		mat[i] = data_p;
		data_p += width;
	}
}

template <typename T>
T* Mat<T>::getRow(int i) const { return mat[i]; }

template <typename T>
T* Mat<T>::getData() const { return data; }

template <typename T>
Mat<T>::Mat(int height, int width)
{
	this->height = height;
	this->width = width;

	this->data = new T[height * width]();
	this->mat = new T*[height];

	for (int i = 0; i < height; i++)
		this->mat[i] = &data[width * i];
}

template <typename T>
Mat<T>::Mat(int height, int width, T* data)
{
	this->height = height;
	this->width = width;

	this->data = new T[height * width]();
	this->mat = new T*[height];

	memcpy(this->data, data, width*height*sizeof(T));

	for (int i = 0; i < height; i++)
		this->mat[i] = &this->data[width * i];
}

template <typename T>
T& Mat<T>::operator()(int row, int col)
{
	return mat[row][col];
}

template <typename T>
Mat<T>::Mat(const Mat<T> &other)
{
	height = other.height;
	width = other.width;

	this->data = new T[height * width]();
	this->mat = new T*[height];

	T *data_p = data;

	for (int i = 0; i < height; i++)
	{
		this->mat[i] = data_p;
		data_p += width;
	}

	memcpy(data, other.data, width*height*sizeof(T));
}

template <typename T>
Mat<T>::~Mat()
{
	delete[] data;
	delete[] mat;
}

template <typename T>
Mat<T>& Mat<T>::operator=(const T &right)
{
	for (int i = 0; i < width * height; i++)
		data[i] = right;

	return *this;
}

template <typename T>
Mat<T>& Mat<T>::operator=(const Mat<T> &right)
{
	if (this == &right)
		return *this;

	delete[] data;
	delete[] mat;

	height = right.height;
	width = right.width;

	this->data = new T[height * width]();
	this->mat = new T*[height];

	for (int i = 0; i < height; i++)
		this->mat[i] = &data[width * i];

	memcpy(this->data, right.data, height * width * sizeof(T));

	return *this;
}

template <typename T>
Mat<T> Mat<T>::operator-(const Mat<T> &right) const
{
	Mat<T> res(height, width);

	if (right.height == 1)
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
				res.mat[i][j] = mat[i][j] - right.mat[0][j];
		}
	}
	else
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
				res.mat[i][j] = mat[i][j] - right.mat[i][j];
		}
	}

	return res;
}

template <typename T>
Mat<T> Mat<T>::operator-(const T &right) const
{
	Mat<T> res(height, width);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			res.mat[i][j] = mat[i][j] - right;
	}

	return res;
}

template <typename T>
Mat<T> Mat<T>::operator-=(const Mat<T> &right)
{
	if (right.height == 1)
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
				mat[i][j] -= right.mat[0][j];
		}
	}
	else
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
				mat[i][j] -= right.mat[i][j];
		}
	}

	return *this;
}

template <typename T>
Mat<T> Mat<T>::operator-=(const T &right)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			mat[i][j] -= right;
	}

	return *this;
}

template <typename T>
void Mat<T>::setRow(int row, std::initializer_list<T> list)
{
	int i = 0;
	for (auto element : list)
	{
		if (i < width)
			(*this)[row][i] = element;

		i++;
	}
}

template <typename T>
void Mat<T>::setColumn(int column, std::initializer_list<T> list)
{
	int i = 0;
	for (auto element : list)
	{
		if (i < height)
			(*this)[i][column] = element;

		i++;
	}
}


template <typename T>
Mat<T> Mat<T>::operator+(const Mat<T> &right) const
{
	Mat<T> res(height, width);

	if (right.height == 1)
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
				res.mat[i][j] = mat[i][j] + right.mat[0][j];
		}
	}
	else if (right.width == 1)
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
				res.mat[i][j] = mat[i][j] + right.mat[i][0];
		}
	}
	else
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
				res.mat[i][j] = mat[i][j] + right.mat[i][j];
		}
	}

	return res;
}

template <typename T>
Mat<T> Mat<T>::operator+(const T &right) const
{
	Mat<T> res(height, width);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			res.mat[i][j] = mat[i][j] + right;
	}

	return res;
}

template <typename T>
Mat<T> Mat<T>::operator+=(const Mat<T> &right)
{
	if (right.height == 1)
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
				mat[i][j] += right.mat[0][j];
		}
	}
	else
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
				mat[i][j] += right.mat[i][j];
		}
	}

	return *this;
}

template <typename T>
Mat<T> Mat<T>::operator+=(const T &right)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			mat[i][j] += right;
	}

	return *this;
}

template <typename T>
Mat<T> Mat<T>::operator*(const Mat<T> &right) const
{
	Mat<T> res(height, right.width);

	if (typeid(T) == typeid(float))
	{
		long int M = height;
		long int N = right.width;
		long int K = width;
		LAPACK::sgemm(&M, &N, &K, (float*)data, (float*)right.data, (float*)res.data);
	}
	else if (typeid(T) == typeid(double))
	{
		long int M = height;
		long int N = right.width;
		long int K = width;
		LAPACK::dgemm(&M, &N, &K, (double*)data, (double*)right.data, (double*)res.data);
	}
	else
	{
		res = 0;
		int rh = res.height;
		int rw = res.width;

		T *ptr_res_row;
		T *ptr_this_row;
		T *ptr_res;

		for (int i = 0; i < rh; i++)
		{
			ptr_res_row = res.getRow(i);
			ptr_this_row = getRow(i);
			for (int j = 0; j < rw; j++)
			{
				ptr_res = ptr_res_row + j;
				for (int k = 0; k < width; k++)
				{
					*ptr_res += ptr_this_row[k] * right.mat[k][j];
				}
			}
		}
	}

	return res;
}

template <typename T>
Mat<T> Mat<T>::operator*(const T &right) const
{
	Mat<T> res(height, width);
	T *ptr, *ptr_res;

	for (int i = 0; i < res.height; i++)
	{
		ptr = getRow(i);
		ptr_res = res.getRow(i);
		for (int j = 0; j < res.width; j++)
			ptr_res[j] = right * ptr[j];

	}

	return res;
}

template <typename T>
float Mat<T>::BLASdot(const Mat<float> &right) const
{
	long int one = 1;
	long int zero = 0;
	float one2 = 1;
	float zero2 = 0;

	long int K = width;

	float res = (float)LAPACK::sdot(&K, data, &one, right.data, &one);

	return res;
}

template <typename T>
Mat<T> Mat<T>::operator/(const T &right) const
{
	Mat<T> res(height, width);

	for (int i = 0; i < res.height; i++)
	{
		for (int j = 0; j < res.width; j++)
		{
			res.mat[i][j] = mat[i][j] / right;
		}
	}

	return res;
}

template <typename T>
Mat<T> Mat<T>::operator/=(const T &right)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			mat[i][j] /= right;
	}

	return *this;
}

template <typename T>
void Mat<T>::subtractColumn(const Mat<T> &right)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			mat[i][j] -= right.mat[0][i];
	}
}


template <typename T>
void Mat<T>::subtractRow(const Mat<T> &right)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			mat[i][j] -= right.mat[0][j];
	}
}

template<typename T>
void Mat<T>::addRow(const Mat<T> &right)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			mat[i][j] += right.mat[0][j];
	}
}

template <typename T>
double Mat<T>::mean()
{
	double sum = 0;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			sum += mat[i][j];
		}
	}

	return sum / (height * width);
}

template <typename T>
Mat<T> Mat<T>::meanRow() const
{
	Mat<T> sum(1, width);
	sum = 0;
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			sum.mat[0][i] += mat[j][i];
		}
		sum.mat[0][i] = sum.mat[0][i] / height;
	}

	return sum;
}

template <typename T>
Mat<T> Mat<T>::minRow() const
{
	Mat<T> minv(1, width);

	if (width == 0 || height == 0)
		return minv;

	for (int i = 0; i < width; i++)
	{
		minv.mat[0][i] = mat[0][i];
		for (int j = 1; j < height; j++)
		{
			T val = mat[j][i];
			if (val < minv.mat[0][i])
				minv.mat[0][i] = val;
		}
	}

	return minv;
}

template <typename T>
Mat<T> Mat<T>::maxRow() const
{
	Mat<T> maxv(1, width);

	if (width == 0 || height == 0)
		return maxv;

	for (int i = 0; i < width; i++)
	{
		maxv.mat[0][i] = mat[0][i];
		for (int j = 1; j < height; j++)
		{
			T val = mat[j][i];
			if (val > maxv.mat[0][i])
				maxv.mat[0][i] = val;
		}
	}

	return maxv;
}

template <typename T>
Mat<T> Mat<T>::clip(T min, T max)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (mat[i][j] > max)
				mat[i][j] = max;
			if (mat[i][j] < min)
				mat[i][j] = min;
		}
	}

	return *this;
}

template <typename T>
double Mat<T>::std()
{
	double meanVal = mean();
	double sum = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			sum += pow(mat[i][j] - meanVal, 2);
		}
	}

	return sqrt(sum / (height * width));
}

template <typename T>
T Mat<T>::norm()
{
	double sum = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			sum += mat[i][j] * mat[i][j];
		}
	}

	return static_cast<float>(sqrt(sum));
}

template <typename T>
T Mat<T>::norm(int start_col, int end_col, int start_row, int end_row)
{
	double sum = 0;
	for (int i = start_row; i < end_row; i++)
	{
		for (int j = start_col; j < end_col; j++)
		{
			sum += mat[i][j] * mat[i][j];
		}
	}

	return static_cast<float>(sqrt(sum));
}

template <typename T>
void Mat<T>::divideFlattened(int start_col, int end_col, int start_row, int end_row, int n_el_in_row, T val)
{
	double sum = 0;
	for (int i = start_row; i < end_row; i++)
	{
		for (int j = start_col; j < end_col; j++)
		{
			int pos = i * n_el_in_row + j;
			data[pos] /= val;
		}
	}
}

template <typename T>
double Mat<T>::flattenNorm(int start_col, int end_col, int start_row, int end_row, int n_el_in_row)
{
	double sum = 0;
	for (int i = start_row; i < end_row; i++)
	{
		for (int j = start_col; j < end_col; j++)
		{
			int pos = i* n_el_in_row + j;
			sum += data[pos] * data[pos];
		}
	}

	return sqrt(sum);
}

template <typename T>
Mat<T> Mat<T>::trans() const
{
	Mat<T> res(width, height);

	for (int i = 0; i < res.height; i++)
	{
		for (int j = 0; j < res.width; j++)
			res.mat[i][j] = mat[j][i];
	}

	return res;
}

template <typename T>
Mat<T> Mat<T>::flatten() const
{
	Mat<T> res(1, height * width);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			res.mat[0][i * width + j] = mat[i][j];
	}

	return res;
}

template <typename T>
Mat<T> Mat<T>::flattenT() const
{
	Mat<T> res(height * width, 1);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			res.mat[i * width + j][0] = mat[i][j];
	}

	return res;
}

template <typename T>
T Mat<T>::maximum() const
{
	T res = mat[0][0];

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (mat[i][j] > res)
				res = mat[i][j];
		}
	}

	return res;
}

template <typename T>
T Mat<T>::minimum() const
{
	T res = mat[0][0];

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (mat[i][j] < res)
				res = mat[i][j];
		}
	}

	return res;
}

template <typename T>
void Mat<T>::print() const
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			std::cout << std::setw(5) << std::setprecision(5) << mat[i][j] << " ";

		std::cout << std::endl;
	}
}

template <typename T>
Mat<T> Mat<T>::subMat(int startRow, int endRow, int startCol, int endCol) const
{
	Mat<T> res(endRow - startRow, endCol - startCol);

	for (int i = startRow, i2 = 0; i < endRow; i++, i2++)
	{
		for (int j = startCol, j2 = 0; j < endCol; j++, j2++)
		{
			res.mat[i2][j2] = mat[i][j];
		}
	}

	return res;
}

template <typename T>
Mat<T> Mat<T>::vstack(Mat<T> other) const
{
	Mat<T> res(height + other.height, width);
	for (int i = 0; i < height + other.height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (i < height)
				res.mat[i][j] = mat[i][j];
			else
				res.mat[i][j] = other.mat[i - height][j];
		}
	}

	return res;
}

template <typename T>
Mat<T> Mat<T>::hstack(Mat<T> other) const
{
	Mat<T> res(height, width + other.width);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width + other.width; j++)
		{
			if (j < width)
				res.mat[i][j] = mat[i][j];
			else
				res.mat[i][j] = other.mat[i][j - width];
		}
	}

	return res;
}

template <typename T>
Mat<float> Mat<T>::inv()
{
	Mat<T> res(*this);

	long int N = height;
	long int *IPIV = new long int[N + 1];
	long int LWORK = N*N;
	float *WORK = new float[LWORK];
	long int INFO;

	LAPACK::sgetrf(&N, &N, res.data, &N, IPIV, &INFO);
	LAPACK::sgetri(&N, res.data, &N, IPIV, WORK, &LWORK, &INFO);

	delete IPIV;
	delete WORK;


	return res;
}

template <typename T>
Mat<float> Mat<T>::solve(Mat<float> &b)
{
	Mat<T> factor(*this);
	Mat<T> res(b);

	long int N = height;
	long int *IPIV = new long int[N + 1];
	long int LWORK = N*N;
	float *WORK = new float[LWORK];
	long int INFO;

	LAPACK::sgetrf(&N, &N, factor.data, &N, IPIV, &INFO);

	char trans = 'T';
	long int nrhs = 1;

	LAPACK::sgetrs(&trans, &N, &nrhs, factor.data, &N, IPIV, res.data, &N, &INFO);

	delete IPIV;
	delete WORK;

	return res;
}

template <typename T>
Mat<double> Mat<T>::solve(Mat<double> &b)
{
	Mat<T> factor(*this);
	Mat<T> res(b);

	long int N = height;
	long int *IPIV = new long int[N + 1];
	long int LWORK = N*N;
	double *WORK = new double[LWORK];
	long int INFO;

	//LAPACK::dgetrf(&N, &N, factor.data, &N, IPIV, &INFO);

	char trans = 'T';
	long int nrhs = 1;

	//LAPACK::dgetrs(&trans, &N, &nrhs, factor.data, &N, IPIV, res.data, &N, &INFO);

	delete IPIV;
	delete WORK;

	return res;
}

template <typename T>
Mat<float> Mat<T>::pinv()
{
	Mat<T> U, D, V;
	Mat<T> res(height, width);

	svd(U, D, V);

	for (int i = 0; i < D.height; i++)
	{
		if (D[i][i] < 0.000001)
			D[i][i] = 0;
		else
			D[i][i] = 1 / D[i][i];
	}

	Mat<int> idxs(min(height, width), 1);
	for (int i = 0; i < min(height, width); i++)
		idxs.data[i] = i;

	V = V.subsetRows(idxs);
	U = U.subsetCols(idxs);

	res = V.trans() * D * U.trans();

	return res;
}

template <typename T>
int Mat<T>::argmin() const
{
	T minVal = mat[0][0];
	int minInd = 0;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (mat[i][j] < minVal)
			{
				minVal = mat[i][j];
				minInd = i * width + j;
			}
		}
	}

	return minInd;
}

template <typename T>
int Mat<T>::argmax() const
{
	T maxVal = mat[0][0];
	int maxInd = 0;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (mat[i][j] > maxVal)
			{
				maxVal = mat[i][j];
				maxInd = i * width + j;
			}
		}
	}

	return maxInd;
}


template <typename T>
Mat<float> Mat<T>::sumRows()
{
	Mat<float> summed(height, 1);

	summed = 0;
	for (int i = 0; i < height; i++)
	for (int j = 0; j < width; j++)
		summed.data[i] += mat[i][j];

	return summed;
}


template <typename T>
T Mat<T>::sum() const
{
	T total = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			total += mat[i][j];
		}
	}

	return total;
}

template <typename T>
Mat<T> Mat<T>::piecewisePow(double power) const
{
	Mat<T> res(height, width);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			res.mat[i][j] = static_cast<T>(pow(mat[i][j], power));
		}
	}

	return res;
}

template <typename T>
void Mat<T>::copyFromUCharTable(unsigned char *src, int _width, int _height)
{
	setSize(_height, _width);
	for (int i = 0; i<width*height; i++)
	{
		data[i] = (T)src[i];
	}
}

template <typename T>
void Mat<T>::copyFromUCharTableWithFlip(unsigned char *src, int _width, int _height)
{
	setSize(_height, _width);
	T* row;
	int pos;
	for (int i = 0; i<height; i++)
	{
		row = getRow(i);
		pos = (height - i - 1)*width;
		for (int j = 0; j<width; j++)
		{
			row[j] = (T)src[pos + j];
		}
	}
}

template <typename T>
void Mat<T>::swapColumns(int i, int j)
{
	T ccopy;
	T* row;
	for (int ri = 0; ri<height; ri++)
	{
		row = getRow(ri);
		ccopy = row[i];
		row[i] = row[j];
		row[j] = ccopy;
	}

}

template <typename T>
void Mat<T>::swapRows(int i, int j)
{
	T* rcopy = new T[width];
	memcpy(rcopy, getRow(i), width*sizeof(T));
	memcpy(getRow(i), getRow(j), width*sizeof(T));
	memcpy(getRow(j), rcopy, width*sizeof(T));

	delete[]rcopy;
}

template <typename T>
Mat<T> Mat<T>::diag()
{
	Mat<T> diag(height, 1);

	for (int i = 0; i < height; i++)
		diag[i][0] = mat[i][i];

	return diag;
}

template <typename T>
float Mat<T>::trace()
{
	float res = 0;

	for (int i = 0; i < height; i++)
	{
		res += mat[i][i];
	}

	return res;
}

template <typename T>
Mat<T> Mat<T>::subsetRows(const int* indexes, int n_indexes)

{
	int size = n_indexes;
	Mat<T> new_mat;
	new_mat.setSize(size, width);

	for (int i = 0; i < size; i++)
		new_mat.copyRow(*this, i, indexes[i]);


	return new_mat;



}

template <typename T>
void Mat<T>::copyFrom(const Mat<T> &source, int dest_row, int dest_col)
{
	for (int row = 0; row < source.height; row++)
		for (int col = 0; col < source.width; col++)
			mat[row + dest_row][col + dest_col] = source.mat[row][col];

}

template <typename T>
Mat<T> Mat<T>::subsetRows(const Mat<int> &indexes)
{
	int size = indexes.width * indexes.height;
	Mat<T> new_mat;
	new_mat.setSize(size, width);


	for (int i = 0; i < size; i++)
		new_mat.copyRow(*this, i, indexes.data[i]);

	return new_mat;
}

template <typename T>
Mat<T> Mat<T>::subsetCols(const Mat<int> &indexes)
{
	int size = indexes.width * indexes.height;
	Mat<T> new_mat;
	new_mat.setSize(height, size);


	for (int i = 0; i < size; i++)
	for (int j = 0; j < height; j++)
		new_mat[j][i] = mat[j][indexes.data[i]];

	return new_mat;
}


template <typename T>
void Mat<T>::svd(Mat<float> &U, Mat<float> &S, Mat<float> &V)
{
	char jobu = 'A';
	char jobvt = 'A';

	long int m = height;
	long int n = width;

	Mat<float> a = this->trans();
	long int lda = height;

	float* arrS = new float[min(m, n)];
	U = Mat<float>(m, m);
	long int ldu = m;

	V = Mat<float>(n, n);
	long int ldvt = n;

	float *work = new float[1];
	long int lwork = -1;
	long int info;

	LAPACK::sgesvd(&jobu, &jobvt, &m, &n, a.data, &lda,
		arrS, U.data, &ldu, V.data, &ldvt,
		work, &lwork, &info);

	lwork = work[0];
	delete[] work;
	work = new float[lwork];

	LAPACK::sgesvd(&jobu, &jobvt, &m, &n, a.data, &lda,
		arrS, U.data, &ldu, V.data, &ldvt,
		work, &lwork, &info);

	delete[] work;

	S = Mat<float>(min(m, n), min(m, n));
	for (int i = 0; i < min(m, n); i++)
	{
		S[i][i] = arrS[i];
	}
	delete[] arrS;

	U = U.trans();
	V = V.trans();
}

template <typename T>
void Mat<T>::writeToOBJ(const char *filename)
{
	if (width != 3) return;

	FILE *f = fopen(filename, "wt");

	for (int i = 0; i < height; i++)
	{
		fprintf(f, "v %.4f %.4f %.4f\n", mat[i][0], mat[i][1], mat[i][2]);
	}

	fclose(f);
}

template <typename T>
bool Mat<T>::readFromPlyFile(const char* filename)
{
	FILE *f = fopen(filename, "rt");

	if (f == NULL) return false;

	char buf[2048];
	int n_elements = 0;

	while (strncmp("end_header", buf, 10))
	{
		fgets(buf, 2048, f);
		if (strncmp("element vertex", buf, 14) == 0)
			sscanf(buf + 15, "%d", &n_elements);
	}

	if (n_elements == 0)
	{
		fclose(f);
		return false;
	}
	setSize(n_elements, 6);
	int n_read = 0;

	while (fgets(buf, 2048, f) != NULL)
	{
		int tmp[3];

		sscanf(buf, "%f %f %f %d %d %d", mat[n_read], mat[n_read] + 1, mat[n_read] + 2,
			tmp, tmp + 1, tmp + 2);
		for (int i = 0; i < 3; i++)
			mat[n_read][i + 3] = tmp[i];
		n_read++;
	}

	fclose(f);
	return true;
}


template <typename T>
bool Mat<T>::readVertexesFromObjFile(const char* filename)
{
	FILE *f = fopen(filename, "rt");

	if (f == NULL) return false;
	char buf[2048];

	int n_vertexes = 0;

	while (fgets(buf, 2048, f) != NULL)
	if (buf[0] == 'v' && buf[1] == ' ')
		n_vertexes++;

	setSize(n_vertexes, 3);
	fseek(f, 0, 0);

	int vertex = 0;
	while (fgets(buf, 2048, f) != NULL)
	if (buf[0] == 'v' && buf[1] == ' ')
	{
		sscanf(buf + 2, "%f %f %f", mat[vertex], mat[vertex] + 1, mat[vertex] + 2);
		vertex++;
	}
	return true;

	fclose(f);
}


template <typename T>
bool Mat<T>::readFromFile(const char* shortname)
{
	string dim_name = string(shortname) + ".dim";
	string data_name = string(shortname) + ".txt";
	FILE *f;

	f = fopen(dim_name.c_str(), "rt");

	if (f == NULL) return false;

	float dim1, dim2;

	fscanf(f, "%f %f", &dim1, &dim2);

	width = (int)dim1;
	height = (int)dim2;

	setSize(width, height);

	fclose(f);

	f = fopen(data_name.c_str(), "rt");
	if (f == NULL) return false;

	for (int i = 0; i < height; i++)
	for (int j = 0; j < width; j++)
	{
		float a = 0;
		fscanf(f, "%f", &a);
		mat[i][j] = static_cast<T>(a);
	}

	fclose(f);
	return true;
}

template <typename T>
Mat<T> Mat<T>::eye(int size)
{
	Mat<T> res(size, size);

	for (int i = 0; i < size; i++)
		res[i][i] = 1;

	return res;
}

template <typename T>
Mat<T> Mat<T>::ones(int height, int width)
{
	Mat<T> res(height, width);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			res[i][j] = 1;
	}

	return res;
}
