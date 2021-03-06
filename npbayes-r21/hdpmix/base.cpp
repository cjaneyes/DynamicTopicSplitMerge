#ifndef _BASE
#define _BASE

/*
 * Structure representing the base distribution, in particular, the
 * parameters for the base distribution, and samples from the base
 * distribution (one sample per class).
 *
 * numclass     The number of samples from the base distribution (number of
 *              clusters in HDP mixture).
 * maxclass     The number of samples for which we have allocated memory.
 *              This is initialized to (numclass+2)*2.
 * hh           The parameters for the base distribution.
 * classqq      The sufficient statistics for data items associated with
 *              each cluster.  In matlab, classqq(:,kk) refers to the
 *              statistics for class kk.  There is numclass+1 represented
 *              clusters (one additional cluster with no data associated
 *              with it).
 * beta         Only used internally.  A vector of numclass 0's following by
 *              one 1.  In matlab this is a row vector.
 *
 * BASE *mxReadBase(mxArray *mstruct);
 *              Reads in a BASE struct from a matlab struct.
 * void mxWriteBase(mxArray *result,BASE *base);
 *              Writes a BASE struct to a matlab struct.  Overwrites fields
 *              in result if necessary.  Frees memory allocated.
 */

#include "../utilities/mxutils.cpp"

typedef struct
{
	int numclass, maxclass, old_numclass;
	HH hh;
	QQ *classqq;
	double *beta;
	double *old_beta;
	int *old_classnt;
	LL *lambda;
} BASE;

BASE *mxReadBase(mxArray *mstruct, unsigned char bEvo)
{
	BASE *result;
	int ii, maxclass;
	result = (BASE*)mxMalloc(sizeof(BASE));
	result->numclass = mxReadScalar(mxReadField(mstruct, "numclass"));
	result->maxclass = maxclass = (result->numclass + 2) * 2;
	result->hh = mxReadHH(mxReadField(mstruct, "hh"));
	result->classqq = mxReadQQVector(result->hh, mxReadField(mstruct, "classqq"),
		result->maxclass);
	result->beta = (double*)mxMalloc(sizeof(double)*maxclass);
	for (ii = 0; ii < maxclass; ii++)
		result->beta[ii] = 0.0;
	result->beta[result->numclass] = 1.0;

	if (bEvo)
	{
		int old_numclass;
		old_numclass = result->old_numclass = mxReadScalar(mxReadField(mstruct, "old_numclass"));
		result->old_beta = mxReadDoubleVector(mxReadField(mstruct, "old_beta"), 0, 0.0, 0.0);
		result->old_classnt = mxReadIntVector(mxReadField(mstruct, "old_classnt"), 0, 0, 0);
		// intialize lambda
		result->lambda = (double**)mxMalloc(sizeof(double *) * maxclass);
        for(ii = 0; ii < maxclass; ii++)
        {
			result->lambda[ii] = (double*)mxMalloc(sizeof(double) * old_numclass);
			int jj;
            for(jj = 0;jj < old_numclass;jj++)
                result->lambda[ii][jj] = 1.0 / old_numclass;
        }
	}

	return result;
}

void mxWriteBase(mxArray *result, BASE *base, unsigned char bEvo)
{
	mxWriteField(result, "numclass", mxWriteScalar(base->numclass));
	mxWriteField(result, "classqq", mxWriteQQVector(base->hh,
		base->numclass + 1, base->maxclass, base->classqq));
	if (bEvo)
	{
		int ii, jj, l = 0;
		double *pr;
		mxArray *mx_lambda = mxCreateDoubleMatrix(base->old_numclass, base->numclass + 1, mxREAL);
		pr = mxGetPr(mx_lambda);
		for (jj = 0; jj < base->maxclass; jj++)
		{
			if (jj <= base->numclass)
				for (ii = 0; ii < base->old_numclass; ii++)
					pr[l++] = base->lambda[jj][ii];
			mxFree(base->lambda[jj]);
		}
		mxFree(base->lambda);
		mxFree(base->old_beta);
		mxFree(base->old_classnt);
		mxWriteField(result, "lambda", mx_lambda);
	}
	mxFreeHH(base->hh);
	mxFree(base->beta);
	mxFree(base);
}

#endif
