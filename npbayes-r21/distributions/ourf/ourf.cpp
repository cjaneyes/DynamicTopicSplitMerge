#pragma once

#include "mex.h"
#include <math.h>
#include <stdlib.h>
#include "../../utilities/mxutils.cpp"

typedef struct
{
	int numdim;
	double *eta;
} HH;

typedef int* SS;
typedef double* LL;
typedef int* QQ;

int numitems(QQ qq)
{
	return qq[0];
}

double marglikelihood(HH hh, QQ qq, SS ss)
{
	//return log((hh.eta[ss] + qq[ss]) / (hh.eta[0] + qq[0]));
    return 0;
}

double marglikelihoods(double *clik, HH hh, int num_topic, QQ *qq, SS ss)
{
	/*double etas, eta0;
	int ii;
	eta0 = hh.eta[0];
	etas = hh.eta[ss];
	for (ii = 0; ii < numqq; ii++)
		clik[ii] = (etas + qq[ii][ss]) / (eta0 + qq[ii][0]);
     */
    
    //return P(x_i^t | Z_i^t = k, x_{-i}^t), it is an array of size num_topic, indexed by cc, in paper indexed by k
    int num_vocab = hh.numdim;
    for(int cc = 0;cc < num_topic;cc++)
    {
        double res;
        if(cc == num_topic-1) //compute f_new
        {
            res = (-1) * lgamma(hh.eta[0] + ss[0]);
            for(int w = 1; w <= num_vocab;w++)
            {
                res += lgamma(hh.eta[w]+ss[w]);
            }
        }
        else
        {
            res = num_topic * lgamma(hh.eta[0]);
            res -= lgamma(hh.eta[0] + qq[cc][0] + ss[0]);
            for(int w = 1;w <= num_vocab;w++)
            {
                res += lgamma(hh.eta[w]+qq[cc][w] + ss[w]) - lgamma(hh.eta[w]);
            }
        }
        clik[cc] = res;
    }
	return 0;
}

void adddata(HH hh, QQ qq, SS ss)
{
	//qq[ss] ++;
	//qq[0] ++;
    int num_vocab = hh.numdim;
    for(int w = 0; w <= num_vocab;w++)
    {
        qq[w] += ss[w];
    }
}

double adddatalik(HH hh, QQ qq, SS ss)
{
	//return log((hh.eta[ss] + (qq[ss] ++)) / (hh.eta[0] + (qq[0] ++)));
    return 0;
}

void deldata(HH hh, QQ qq, SS ss)
{
	//qq[ss] --;
	//qq[0] --;
    int num_vocab = hh.numdim;
    for(int w = 0; w <= num_vocab;w++)
    {
        qq[w] -= ss[w];
    }
	if (qq[0] < 0)
		qq[0] = qq[0];
}

QQ newclass(HH hh)
{
	int ii;
	QQ result;
	result = (int*)mxMalloc(sizeof(int)*(hh.numdim + 1));
	memset(result, 0, sizeof(int)*(hh.numdim + 1));
	return result;
}

SS *mxReadSSVector(const mxArray *mvector)
{
	SS *result;
	double *pr;
	int i, j, l = 0;
	unsigned m, n;
	pr = mxGetPr(mvector);
	m = mxGetM(mvector);
	n = mxGetN(mvector);
	
	result = (SS*)mxMalloc(sizeof(SS)*n);
	for (j = 0; j < n; j++)
	{
		int sum = 0;
		result[j] = (int*)mxMalloc(sizeof(int)*(m + 1));
		for (i = 1; i <= m; i++)
			sum += (result[j][i] = pr[l++]);
		result[j][0] = sum;
	}
	return result;
}

mxArray *mxWriteSSVector(int numss, SS *ss)
{
	//mxArray *result;
	//double *pr;
	//int ii;
	//result = mxCreateDoubleMatrix(1, numss, mxREAL);
	//pr = mxGetPr(result);
	//for (ii = 0; ii < numss; ii++)
	//	pr[ii] = ss[ii];
	//mxFree(ss);
	//return result;
	return 0;
}

void mxFreeSSVector(int numss, SS *ss)
{
	int i;
	for (i = 0; i < numss; i++)
		mxFree(ss[i]);
}

HH mxReadHH(const mxArray *mvector)
{
	double *pr, sum;
	int ii, jj;
	HH result;
	result.numdim = mxGetNumberOfElements(mvector);
	result.eta = (double*)mxMalloc(sizeof(double)*(1 + result.numdim));
	sum = 0.0;
	pr = mxGetPr(mvector);
	for (ii = 0; ii < result.numdim; ii++)
		sum += result.eta[ii + 1] = pr[ii];
	result.eta[0] = sum;
	return result;
}

void mxFreeHH(HH hh)
{
	mxFree(hh.eta);
}

QQ *mxReadQQVector(HH hh, const mxArray *marray, int maxnum)
{
	double *pr;
	int ii, jj, mm, nn, sum, l = 0;
	QQ *result;
	mm = mxGetM(marray);
	nn = mxGetN(marray);
	if (mm != hh.numdim) mexErrMsgTxt("Number of dimensions don't match.");
	maxnum = max(maxnum, nn);
	result = (QQ*)mxMalloc(sizeof(QQ)*maxnum);
	pr = mxGetPr(marray);
	for (jj = 0; jj < nn; jj++)
	{
		result[jj] = newclass(hh);
		sum = 0;
		for (ii = 1; ii <= mm; ii++)
			sum += (result[jj][ii] = pr[l++]);
		result[jj][0] = sum;
	}
	for (jj = nn; jj < maxnum; jj++)
		result[jj] = newclass(hh);
	return result;
}

mxArray *mxWriteQQVector(HH hh, int nn, int maxnum, QQ *qq)
{
	mxArray *result;
	double *pr;
	int ii, jj, l = 0;
	result = mxCreateDoubleMatrix(hh.numdim, nn, mxREAL);
	pr = mxGetPr(result);
	mxdebug2(3, "Write qq (%dx%d): ", hh.numdim, nn);
	for (jj = 0; jj < nn; jj++)
	{
		for (ii = 1; ii <= hh.numdim; ii++)
			pr[l++] = qq[jj][ii];
		mxFree(qq[jj]);
		mxdebug1(4, "%d ", jj);
	}
	mxdebug0(3, "Free qq: ");
	for (jj = nn; jj < maxnum; jj++)
	{
		mxFree(qq[jj]);
		mxdebug1(4, "%d ", jj);
	}
	mxFree(qq);
	mxdebug0(3, "\n");
	return result;
}

