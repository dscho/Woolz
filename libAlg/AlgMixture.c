#pragma ident "MRC HGU $Id$"
/************************************************************************
* Project:      Mouse Atlas
* Title:        AlgMixture.c
* Date:         January 2000
* Author:       Bill Hill
* Copyright:	2000 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:	Provides a function for computing the maximum liklihood
*		parameters of a mixture of distributions which fit the
*		given data for the MRC Human Genetics Unit numerical
*		algorithm library.
* $Revision$
* Maintenance:  Log changes below, with most recent at top of list.
************************************************************************/
#include <Alg.h>
#include <math.h>
#include <float.h>

/************************************************************************
* Function:	AlgMixtureMLG
* Returns:	AlgError:		Error code.
* Purpose:	Computes the maximum liklihood estimate of the
*		parameters of a mixture of normal distributions which
*		best fit the give frequencies. These parameters are the
*		miximg proportions, means, and standard deviations.
*		The log-liklihood and the number of iterations are also
*		computed.
*		This algorithm is based on the article:
*		Agha M. and Ibrahim M.T.
*		Maximum Liklihood Estimation of Mixtures of Distributions
*	 	AS203 Applied Statistics 33(3):327-332, 1984.
* Global refs:	-
* Parameters:	int nDbn:		Number of distributions in the
*					mixture.
*		int nCls:		Number of classes in the
*					frequency distribution.
*		double samOrg:		Position of first sample (origin).
*		double samItv:		Sample interval.
*		double *freq:		Given frequency samples.
*		double *alpha:		Estimates of nDbn alpha values,
*					both given and returned.
*				 	Alpha values must be in the
*					range (0,1].
*		double *mu:		Estimates of nDbn mean values,
*					both given and returned.
*		double *sd:		Estimates of nDbn standard
*					deviation values,
*					both given and returned.
*		double tol:		Difference between two
*					consecutive log liklihood values
*					required to terminate iteration.
*		int sumFreq:		Number of observations.
*		double *dstLL:		Destination pointer for the
*					log liklihood.
*		double *dstNItn:	Destination pointer for the
*					number of iterations taken.
************************************************************************/
AlgError	AlgMixtureMLG(int nDbn, int nCls,
			      double samOrg, double samItv, double *freq,
			      double *alpha, double *mu, double *sd,
			      double tol, double sumFreq,
			      double *dstLL, int *dstNItn)
{
  int		idC,
  		idD,
		idD1,
		converged;
  double	tD0,
  		tD1,
		tD2,
		oldLL,
		sumAlpha;
  double	*g = NULL,
  		*vt = NULL,
		*dt = NULL,
		*nt = NULL,
		*newalpha = NULL,
		*newmean = NULL,
		*newsd = NULL;
  double	**f = NULL;
  AlgError	algErr = ALG_ERR_NONE;

  /* Check parameters */
  if((nDbn < 2) || (nDbn > (nCls / 2)) || (samItv < DBL_EPSILON))
  {
    algErr = ALG_ERR_FUNC;
  }
  if(algErr == ALG_ERR_NONE)
  {
    if(nCls < (nDbn * 2))
    {
      algErr = ALG_ERR_FUNC;
    }
  }
  if(algErr == ALG_ERR_NONE)
  {
    for(idC = 0; idC < nCls; ++idC)
    {
      if(freq[idC] < 0.0)
      {
	algErr = ALG_ERR_FUNC;
      }
    }
  }
  if(algErr == ALG_ERR_NONE)
  {
    /* Allocate temporary storage. */
    if((AlcDouble2Malloc(&f, nCls, nDbn) != ALC_ER_NONE) ||
       ((newalpha = (double *)AlcMalloc(sizeof(double) * nDbn)) == NULL) ||
       ((newmean = (double *)AlcMalloc(sizeof(double) * nDbn)) == NULL) ||
       ((newsd = (double *)AlcMalloc(sizeof(double) * nDbn)) == NULL) ||
       ((vt = (double *)AlcMalloc(sizeof(double) * nDbn)) == NULL) ||
       ((dt = (double *)AlcMalloc(sizeof(double) * nDbn)) == NULL) ||
       ((nt = (double *)AlcMalloc(sizeof(double) * nDbn)) == NULL) ||
       ((g = (double *)AlcMalloc(sizeof(double) * nCls)) == NULL))
    {
      algErr = ALG_ERR_MALLOC;
    }
  }
  if(algErr == ALG_ERR_NONE)
  {
    /* The iteration loop. */
    oldLL = 0.0;
    *dstNItn = 0;
    do
    {
      ++*dstNItn;
      /* Check alpha, mu and standard deviation. */
      for(idD = 0; idD < nDbn; ++idD)
      {
	if(isnan(alpha[idD]) ||
	   (alpha[idD]) > 1.0 || (alpha[idD] < 0.0))
	{
	  algErr = ALG_ERR_CONVERGENCE;
	}
	else if(isnan(mu[idD]) ||
		(mu[idD] >= samOrg + (samItv * (nCls - 1))) ||
		(mu[idD] <= samOrg))
	{
	  algErr = ALG_ERR_CONVERGENCE;
	}
      }
      if(algErr == ALG_ERR_NONE)
      {
	for(idD = 0; idD < nDbn; ++idD)
	{
	  if(isnan(sd[idD]) || (sd[idD] <= 0.0))
	  {
	    algErr = ALG_ERR_CONVERGENCE;
	  }
	}
      }
      if(algErr == ALG_ERR_NONE)
      {
	idD = 0;
	while((idD < nDbn - 1) && (algErr == ALG_ERR_NONE))
	{
	  idD1 = idD + 1;
	  while((idD1 < nDbn) && (algErr == ALG_ERR_NONE))
	  {
	    if(mu[idD] == mu[idD1])
	    {
	      if(sd[idD] == sd[idD1])
	      {
		algErr = ALG_ERR_FUNC;
	      }
	    }
	    ++idD1;
	  }
	  ++idD;
	}
      }
      if(algErr == ALG_ERR_NONE)
      {

	*dstLL = 0.0;
	for(idC = 0; idC < nCls; ++idC)
	{
	  g[idC] = 0.0;
	  for(idD = 0; idD < nDbn; ++idD)
	  {
	    tD0 = (samOrg + (samItv * idC) - mu[idD]) / sd[idD];
	    tD1 = exp(-0.5 * (tD0 * tD0)) / sd[idD];
	    *(*(f + idC) + idD) = tD1;
	    g[idC] += alpha[idD] * tD1;
	  }
	  /* Avoid numerical problems (eg log(0.0)) by adding some noise. */
	  g[idC] += (drand48() * 1.0e-6) +  1.0e-8;
	  *dstLL += freq[idC] * log(g[idC]);
	}
	/* Calculate the probability densities of the sub-populations
	 * which form the mixture, and the log-likelihood function.  */
	converged = 0;
	sumAlpha = 0.0;
	for(idD = 0; idD < nDbn; ++idD)
	{
	  nt[idD] = dt[idD] = vt[idD] = 0.0;
	  for(idC = 0; idC < nCls; ++idC)
	  {
	    tD0 = samOrg + (samItv * idC);
	    tD1 = *(*(f + idC) + idD) * freq[idC] / g[idC];
	    dt[idD] += tD1;
	    nt[idD] += tD0 * tD1;
	    tD2 = tD0 - mu[idD];
	    vt[idD] += tD1 * tD2 * tD2;
	  }
	  /* Calculate denominators and numerators of new estimates. */
	  newmean[idD] = nt[idD] / dt[idD];
	  newalpha[idD] = alpha[idD] * dt[idD] / sumFreq;
	  sumAlpha += newalpha[idD];
	  newsd[idD] = sqrt(vt[idD] / dt[idD]);
	  /* Convergence test. */
	  converged = fabs(oldLL - *dstLL) <= tol;
	  oldLL = *dstLL;
	  alpha[idD] = newalpha[idD];
	  mu[idD] = newmean[idD];
	  sd[idD] = newsd[idD];
	}
	/* Ensure alpha's are normalised. */
	tD0 = 1.0 / sumAlpha;
	for(idD = 0; idD < nDbn; ++idD)
	{
	  alpha[idD] *= tD0;
	}
      }
    }
    while(!converged && (algErr == ALG_ERR_NONE));
  }
  /* Free temporary storage. */
  if(f)
  {
    AlcDouble2Free(f);
  }
  if(newalpha)
  {
    AlcFree(newalpha);
  }
  if(newmean)
  {
    AlcFree(newmean);
  }
  if(newsd)
  {
    AlcFree(newsd);
  }
  if(vt)
  {
    AlcFree(vt);
  }
  if(nt)
  {
    AlcFree(nt);
  }
  if(g)
  {
    AlcFree(g);
  }
  return(algErr);
}

/************************************************************************
* Function:	AlgMixtureSyn
* Returns:	AlgError:		Error code.
* Purpose:	Synthesise a mixture of normal distributions.
* Global refs:	-
* Parameters:	
*		int nCls:		Number of classes in the
*					frequency distribution.
*		int *synFreq:		Buffer for synthesised
*					frequencies.
*		int nObv:		Number of observations.
*		double synOrg:		Origin of synFreq buffer.
*		double synStep:		Sample interval for the synFreq
*					buffer.
*		int nDbn:		Number of distributions in the
*					mixture.
*		double *alpha:		Estimates of nDbn alpha values.
*		double *mu:		Estimates of nDbn mu values.
*		double *sigma:		Estimates of nDbn standard
*					deviation values.
************************************************************************/
AlgError	AlgMixtureSyn(int nCls, int *synFreq, int nObv,
			      double synOrg, double synStep,
			      int nDbn,
			      double *alpha, double *mu, double *sigma)
{
  int		idC,
  		idD;
  double	tD0,
  		synPos,
  		synVal,
		synTot;
  AlgError	algErr = ALG_ERR_NONE;

  if((synFreq == NULL) || 
     (alpha == NULL) || (mu == NULL) || (sigma == NULL) ||
     (nCls <= 0) || (nDbn <= 0))
  {
    algErr = ALG_ERR_FUNC;
  }
  else
  {
    synTot = 0.0;
    for(idC = 0; idC < nCls; ++idC)
    {
      synVal = 0.0;
      synPos = synOrg + (synStep * idC);
      for(idD = 0; idD < nDbn; ++idD)
      {
	tD0 = (synPos - *(mu + idD)) / *(sigma + idD);
	synVal +=  *(alpha + idD) *
		   exp(-0.5 * tD0 * tD0) / sqrt(*(sigma + idD));
      }
      synTot += *(synFreq + idC) = ALG_NINT(synVal);
    }
    if(synTot > DBL_EPSILON)
    {
      tD0 = nObv / synTot;
      for(idC = 0; idC < nCls; ++idC)
      { 
	synVal = tD0 * *(synFreq + idC);
	*(synFreq + idC) = ALG_NINT(synVal);
      }
    }
  }
  return(algErr);
}

#ifdef ALG_MIXTURE_TEST
int		main(int argc, char **argv)
{
  int		idx,
		ok,
		done,
		ioCnt,
		iMax = 1000,
  		nCls,
  		nDbn,
		nItn;
  double	dataOrg,
  		dataItv,
		tol = 0.001,
		logL;
  FILE		*fP = NULL;
  AlgError	algErr = ALG_ERR_NONE;
  static char	ioBuf[256];
  static double	freq[256],
  		alpha[4],
		mu[4],
		sigma[4];

  ok = 1;
  /* Parse command line. */
  if(argc != 2)
  {
    (void )fprintf(stderr,
    		   "Usage: %s <file>\n", 
		   *argv);
    ok = 0;
  }
  if(ok)
  {
    if((fP = fopen(*(argv + 1), "r")) == NULL)
    {
      (void )fprintf(stderr,
      		     "%s: Failed to open input file %s\n",
		     *argv, *(argv + 1));
      ok = 0;
    }
  }
  if(ok)
  {
    /* Read initial estimates of alpha, mu and sigma until blank line. */
    idx = 0;
    done = 0;
    while(!done && (idx < 256) && (fgets(ioBuf, 256, fP) != NULL))
    {
      ioBuf[255] = '\0';
      if(sscanf(ioBuf, "%lg %lg %lg\n",
      		alpha + idx, mu + idx, sigma + idx) != 3)
      {
        done = 1;
	nDbn = idx;
      }
      else
      {
        ++idx;
      }
    }
    if(done == 0)
    {
      (void )fprintf(stderr,
		     "%s: Failed to read input distribution parameters.\n",
		     *argv);
      ok = 0;
    }
  }
  if(ok)
  {
    /* Read data origin and step size followed by blank line. */
    if((fgets(ioBuf, 256, fP) == NULL) ||
       (sscanf(ioBuf, "%lg %lg", &dataOrg, &dataItv) != 2))
    {
      (void )fprintf(stderr, 
      		     "%s: Failed to read data origin and interval.\n",
		     *argv);
      ok = 0;
    }
    else
    {
      /* Consume blank line. */
      (void )fgets(ioBuf, 256, fP);
    }
  }
  if(ok)
  {
    /* Read data f(x) until end of file */
    idx = 0;
    while((fgets(ioBuf, 256, fP) != NULL) &&
    	  (sscanf(ioBuf, "%lg", freq + idx) == 1))
    {
      ++idx;
    }
    nCls = idx;
  }
  if(ok)
  {
    /* Compute ML Gaussian mixture */
    algErr = AlgMixtureMLG(nDbn, nCls, dataOrg, dataItv, freq,
    			   alpha, mu, sigma,
			   tol, iMax, &logL, &nItn);
    if(algErr != ALG_ERR_NONE)
    {
      (void )fprintf(stderr,
      		     "%s: Failed to compute mixture of Gaussians (%d).\n",
		     *argv, (int )algErr);
      ok = 0;
    }
  }
  if(ok)
  {
    /* Output new estimate of Gaussian mu, sigma and alpha */
    for(idx = 0; idx < nDbn; ++idx)
    {
      (void )printf("%8g %8g %8g %d %8g\n",
      		    alpha[idx], mu[idx], sigma[idx], nItn, logL);
	     
    }
  }
  if(fP)
  {
    (void )fclose(fP);
  }
}
#endif /* ALG_MIXTURE_TEST */