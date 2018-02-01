#pragma once

namespace LAPACK
{
	double sdot(long int *n, float *sx, long int *incx, float *sy, long int *incy);
	void sgemm(long int *m, long int *n, long int *k, float *a, float *b, float *c, char transA = 'N', char transB = 'N');
	void dgemm(long int *m, long int *n, long int *k, double *a, double *b, double *c, char transA = 'N', char transB = 'N');
	void dgemm(long int *m, long int *n, long int *k, double *a, double *b, double *c, char transA, char transB, long int *lda, long int *ldb, long int *ldc);

	void sgetrf(long int *n, long int *m, float *a, long int *lda, long int *ipiv, long int *info);
	void dgetrf(long int *n, long int *m, double *a, long int *lda, long int *ipiv, long int *info);
	void sgetri(long int *n, float *a, long int *lda, long int *ipiv, float *work, long int *lwork, long int *info);
	void dgetri(long int *n, double *a, long int *lda, long int *ipiv, double *work, long int *lwork, long int *info);

	void sgesvd(char *jobu, char *jobvt, long int *m, long int *n, float *a, long int *lda,
		float *s, float *u, long int *ldu, float *vt, long int *ldvt,
		float *work, long int *lwork, long int *info);

	void sgetrs(char *trans, long int *n, long int *nrhs, float *a, long int *lda,
		long int *ipiv, float *b, long int *ldb, long int *info);
	void dgetrs(char *trans, long int *n, long int *nrhs, double *a, long int *lda,
		long int *ipiv, double *b, long int *ldb, long int *info);
}