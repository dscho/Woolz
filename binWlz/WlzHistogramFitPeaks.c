#ifndef DOXYGEN_SHOULD_SKIP_THIS
#pragma ident "MRC HGU $Id$"
/***********************************************************************
* Project:      Woolz
* Title:        WlzHistogramGauss.c
* Date:         February 2000
* Author:       Bill Hill
* Copyright:	2000 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:      Woolz filter which fits a series of Gaussian distributions
*		to a Woolz histogram.
* $Revision$
* Maintenance:	Log changes below, with most recent at top of list.
************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <Wlz.h>
#include <float.h>

extern int      getopt(int argc, char * const *argv, const char *optstring);
 
extern char     *optarg;
extern int      optind,
                opterr,
                optopt;

int             main(int argc, char **argv)
{
  int		idB,
  		idD,
  		option,
		pkCnt = 0,
		numDbn = 0,
		synthesize = 0,
		fitCnt = 0,
		ok = 1,
		usage = 0;
  double	tD0,
		tD1,
		tD2,
		pos,
  		mu,
  		sigma,
		alpha,
		binCnt,
		sumArea,
  		pkSigma = 1.0,
  		pkThresh = 1.0,
		fitTol = 0.1;
  double	*fitMu = NULL,
  		*fitSigma = NULL,
  		*fitAlpha = NULL,
		*fitArea = NULL,
		*synBuf = NULL;
  int		*pkPos = NULL;
  WlzHistogramDomain *inDom,
  		     *outDom;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  FILE		*fP = NULL;
  WlzObject	*inObj = NULL,
  		*outObj = NULL;
  char 		*outDatFileStr,
  		*inObjFileStr;
  const char	*errMsg;
  static char	optList[] = "l:n:o:s:t:hy",
		outDatFileStrDef[] = "-",
  		inObjFileStrDef[] = "-";

  opterr = 0;
  outDatFileStr = outDatFileStrDef;
  inObjFileStr = inObjFileStrDef;
  while(ok && ((option = getopt(argc, argv, optList)) != -1))
  {
    switch(option)
    {
      case 'l':
	if(sscanf(optarg, "%lg", &fitTol) != 1)
	{
	  usage = 1;
	  ok = 0;
	}
	break;
      case 'n':
	if(sscanf(optarg, "%d", &numDbn) != 1)
	{
	  usage = 1;
	  ok = 0;
	}
	break;
      case 'o':
        outDatFileStr = optarg;
	break;
      case 's':
	if(sscanf(optarg, "%lg", &pkSigma) != 1)
	{
	  usage = 1;
	  ok = 0;
	}
	break;
      case 't':
	if(sscanf(optarg, "%lg", &pkThresh) != 1)
	{
	  usage = 1;
	  ok = 0;
	}
	break;
      case 'y':
	synthesize = 1;
        break;
      case 'h':
      default:
        usage = 1;
	ok = 0;
	break;
    }
  }
  if((inObjFileStr == NULL) || (*inObjFileStr == '\0') ||
     (outDatFileStr == NULL) || (*outDatFileStr == '\0'))
  {
    ok = 0;
    usage = 1;
  }
  if(ok && (optind < argc))
  {
    if((optind + 1) != argc)
    {
      usage = 1;
      ok = 0;
    }
    else
    {
      inObjFileStr = *(argv + optind);
    }
  }
  if(ok)
  {
    errNum = WLZ_ERR_READ_EOF;
    if((inObjFileStr == NULL) ||
       (*inObjFileStr == '\0') ||
       ((fP = (strcmp(inObjFileStr, "-")?
	      fopen(inObjFileStr, "r"): stdin)) == NULL) ||
       ((inObj= WlzAssignObject(WlzReadObj(fP, &errNum), NULL)) == NULL))
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
		     "%s: failed to read object from file %s (%s).\n",
		     *argv, inObjFileStr, errMsg);
    }
    if(fP && strcmp(inObjFileStr, "-"))
    {
      fclose(fP);
    }
  }
  if(ok)
  {
    if(inObj->type != WLZ_HISTOGRAM)
    {
      ok = 0;
      (void )fprintf(stderr,
      		     "%s: input object read from file %s not a histogram\n",
		     *argv, inObjFileStr);
    }
  }
  if(ok)
  {
    if(inObj == NULL)
    {
      errNum = WLZ_ERR_OBJECT_NULL;
    }
    else if(inObj->type != WLZ_HISTOGRAM)
    {
      errNum = WLZ_ERR_OBJECT_TYPE;
    }
    else if((inDom = inObj->domain.hist) == NULL)
    {
      errNum = WLZ_ERR_DOMAIN_NULL;
    }
    else if((inDom->nBins < 0) || (inDom->maxBins < 0) ||
    	    (inDom->binSize <= DBL_EPSILON))
    {
      errNum = WLZ_ERR_DOMAIN_DATA;
    }
    else if((inDom->type != WLZ_HISTOGRAMDOMAIN_INT) &&
            (inDom->type != WLZ_HISTOGRAMDOMAIN_FLOAT))
    {
      errNum = WLZ_ERR_DOMAIN_TYPE;
    }
  }
  if(ok)
  {
    errNum = WlzHistogramFitPeaks(inObj, numDbn,
    				  pkSigma, pkThresh, fitTol,
				  &fitCnt, &fitMu,
				  &fitCnt, &fitSigma,
				  &fitCnt, &fitAlpha,
				  NULL);
    if(errNum != WLZ_ERR_NONE)
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
      		     "%s: failed to fit histogram (%s).\n",
		     *argv, errMsg);
    }
  }
  if(ok && (inDom->nBins > 0) && (fitCnt > 0))
  {
    if(((fitArea = (double *)AlcMalloc(fitCnt *
    				      sizeof(double))) == NULL) ||
       ((synBuf = (double *)AlcCalloc(inDom->nBins,
    				      sizeof(double))) == NULL))
    {
      errNum = WLZ_ERR_MEM_ALLOC;
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
      		     "%s: failed to fit histogram (%s).\n",
		     *argv, errMsg);
    }
  }
  if(ok && (fitCnt > 0))
  {
    binCnt = WlzHistogramBinSum(inDom);
    for(idD = 0; idD < fitCnt; ++idD)
    {
      mu = *(fitMu + idD);
      sigma = *(fitSigma + idD);
      alpha = *(fitAlpha + idD);
      tD0 = alpha / (sqrt(2.0 * WLZ_M_PI) * sigma);
      for(idB = 0; idB < inDom->nBins; ++idB)
      {
        pos = inDom->origin + (idB * inDom->binSize) - mu;
	tD1 = pos / sigma;
	tD1 = -0.5 * (tD1 * tD1);
	tD2 = tD0 * exp(tD1);
	*(synBuf + idB) += tD2;
	*(fitArea + idD) += tD2 * inDom->binSize;
	sumArea += tD2 * inDom->binSize;
      }
    }
    tD0 = binCnt / sumArea;
    for(idD = 0; idD < fitCnt; ++idD)
    {
      *(fitArea + idD) *= tD0;
    }
    for(idB = 0; idB < inDom->nBins; ++idB)
    {
      *(synBuf + idB) *= tD0;
    }
    if((fP = (strcmp(outDatFileStr, "-")?
	     fopen(outDatFileStr, "w"): stdout)) == NULL)
    {
      ok = 0;
      (void )fprintf(stderr,
		     "%s: failed to write output data\n",
		     *argv);
    }
    else
    {
      if(synthesize)
      {
        outObj = WlzMakeHistogram(inDom->type, inDom->nBins, &errNum);
	if(errNum != WLZ_ERR_NONE)
	{
	  ok = 0;
	  (void )WlzStringFromErrorNum(errNum, &errMsg);
	  (void )fprintf(stderr,
			 "%s: failed to synthesize histogram (%s).\n",
			 *argv, errMsg);
	}
	else
	{
	  outDom = outObj->domain.hist;
	  outDom->nBins = inDom->nBins;
	  outDom->origin = inDom->origin;
	  outDom->binSize = inDom->binSize;
	  switch(inDom->type)
	  {
	    case WLZ_HISTOGRAMDOMAIN_INT:
	      WlzValueCopyDoubleToInt(outDom->binValues.inp, synBuf,
	      			      inDom->nBins);
	      break;
	    case WLZ_HISTOGRAMDOMAIN_FLOAT:
	      WlzValueCopyDoubleToDouble(outDom->binValues.dbp, synBuf,
	      			         inDom->nBins);
	      break;
	  }
	  if(WlzWriteObj(fP, outObj) != WLZ_ERR_NONE)
	  {
	    ok = 0;
	    (void )fprintf(stderr,
	    		   "%s: failed to write output object\n",
			   *argv);
	  }
	}
      }
      else
      {
	if(fitCnt > 0)
	{
	  for(idD = 0; idD < fitCnt; ++idD)
	  {
	    fprintf(fP, "%8g %8g %8g %8g\n",
		    *(fitMu + idD), *(fitSigma + idD),
		    *(fitAlpha + idD), *(fitArea + idD));
	  }
	}
      }
      if(strcmp(outDatFileStr, "-"))
      {
	fclose(fP);
      }
    }
  }
  if(inObj)
  {
    WlzFreeObj(inObj);
  }
  if(outObj)
  {
    WlzFreeObj(outObj);
  }
  if(fitArea)
  {
    AlcFree(fitArea);
  }
  if(synBuf)
  {
    AlcFree(synBuf);
  }
  if(fitMu)
  {
    AlcFree(fitMu);
  }
  if(fitSigma)
  {
    AlcFree(fitSigma);
  }
  if(fitAlpha)
  {
    AlcFree(fitAlpha);
  }
  if(usage)
  {
    (void )fprintf(stderr,
    "Usage: %s%sExample: %s%s",
    *argv,
    " [-h] [-y] [-o<out file>] [-s#] [-t#] [-l#] [<in object>]\n"
    "Options:\n"
    "  -l  Log-liklihood fitting tolerance.\n"
    "  -n  Number of Gaussian distributions.\n"
    "  -o  Output data file name.\n"
    "  -s  Minimum Gaussian sigma (standard deviation).\n"
    "  -t  Threshold value for peak height.\n"
    "  -y  Synthesize a new histogram from the mixture of Gaussians, by\n"
    "      default just the statistics are output.\n"
    "  -h  Help, prints this usage message.\n"
    "Fits a mixture of Gaussian distributions to the given Woolz histogram"
    "using a maximum liklihood estimate. If given the number of Gaussian\n"
    "distributions is zero (default) then Gaussians will be fitted to\n"
    "peaks found using WlzHistogramFindPeaks.\n"
    "Either the statistics of the Gaussian mixture are output in the format\n"
    "  <mu (mean)> <sigma (std dev)> <alpha (height)> <area>\n"
    "one record for each fitted Gaussian, or a synthesized histogram built\n"
    "using the statistics is output.\n"
    "Objects/data are read from stdin and written to stdout unless the\n"
    "filenames are given.\n",
    *argv,
    " -s1.0 -t 10.0 myhist.wlz\n"
    "The input Woolz histogram object is read from myhist.wlz. Histogram\n"
    "peaks are detected using a Gaussian smoothing filter with a sigma value\n"
    "of 1.0 and a threshold value of 10.0, the maximum liklihood mixture\n"
    "of Gaussians is computed and the mixture statistics are written to the\n"
    "standard output.\n");
  }
  return(!ok);
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
