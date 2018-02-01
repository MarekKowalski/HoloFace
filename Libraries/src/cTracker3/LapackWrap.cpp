#include "LapackWrap.h"
#include "clapack/f2c.h"
#include "clapack/clapack.h"

namespace LAPACK
{
	double sdot(long int *n, float *sx, long int *incx, float *sy, long int *incy)
	{
		return sdot_(n, sx, incx, sy, incy);
	}

	void sgemm(long int *m, long int *n, long int *k, float *a, float *b, float *c, char transA, char transB)
	{
		float alpha = 1.0;
		float beta = 0.0;
		sgemm_(&transA, &transB, n, m, k, &alpha, b, n, a, k, &beta, c, n);
	}

	void dgemm(long int *m, long int *n, long int *k, double *a, double *b, double *c, char transA, char transB)
	{
		double alpha = 1.0;
		double beta = 0.0;
		dgemm_(&transA, &transB, n, m, k, &alpha, b, n, a, k, &beta, c, n);
	}

	void dgemm(long int *m, long int *n, long int *k, double *a, double *b, double *c, char transA, char transB, long int *lda, long int *ldb, long int *ldc)
	{
		double alpha = 1.0;
		double beta = 0.0;
		dgemm_(&transA, &transB, m, n, k, &alpha, a, lda, b, ldb, &beta, c, ldc);
	}

	void sgetrf(long int *n, long int *m, float *a, long int *lda, long int *ipiv, long int *info)
	{
		sgetrf_(m, n, a, lda, ipiv, info);
	}

	void dgetrf(long int *n, long int *m, double *a, long int *lda, long int *ipiv, long int *info)
	{
		dgetrf_(m, n, a, lda, ipiv, info);
	}

	void sgetri(long int *n, float *a, long int *lda, long int *ipiv, float *work, long int *lwork, long int *info)
	{
		sgetri_(n, a, lda, ipiv, work, lwork, info);
	}

	void dgetri(long int *n, double *a, long int *lda, long int *ipiv, double *work, long int *lwork, long int *info)
	{
		dgetri_(n, a, lda, ipiv, work, lwork, info);
	}

	void sgesvd(char *jobu, char *jobvt, long int *m, long int *n, float *a, long int *lda,
		float *s, float *u, long int *ldu, float *vt, long int *ldvt,
		float *work, long int *lwork, long int *info)
	{
		sgesvd_(jobu, jobvt, m, n, a, lda,
			s, u, ldu, vt, ldvt,
			work, lwork, info);
	}

	void sgetrs(char *trans, long int *n, long int *nrhs, float *a, long int *lda,
		long int *ipiv, float *b, long int *ldb, long int *info)
	{
		sgetrs_(trans, n, nrhs, a, lda, ipiv, b, ldb, info);
	}

	void dgetrs(char *trans, long int *n, long int *nrhs, double *a, long int *lda,
		long int *ipiv, double *b, long int *ldb, long int *info)
	{
		dgetrs_(trans, n, nrhs, a, lda, ipiv, b, ldb, info);
	}
}