#pragma ident "MRC HGU $Id$"
/***********************************************************************
* Project:      Woolz
* Title:        WlzHistogramRebin.c
* Date:         March 1999
* Author:       Bill Hill
* Copyright:	1999 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:      Woolz filter which re-bins a Woolz histogram object.
* $Revision$
* Maintenance:	Log changes below, with most recent at top of list.
************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <Wlz.h>

extern int      getopt(int argc, char * const *argv, const char *optstring);
 
extern char     *optarg;
extern int      optind,
                opterr,
                optopt;

int             main(int argc, char **argv)
{
  int		idx,
  		option,
		nBins,
		asciiFlag = 0,
		typeFlag = 0,
		nBinsFlag = 0,
		originFlag = 0,
		widthFlag = 0,
		ok = 1,
		usage = 0;
  WlzObjectType	binType;
  double	binOrigin = 0.0,
  		binWidth = 0.0;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  FILE		*fP = NULL;
  WlzHistogramDomain *histDom;
  WlzObject	*inObj = NULL,
		*outObj = NULL;
  char 		*outObjFileStr,
  		*inObjFileStr;
  const char	*errMsg;
  static char	optList[] = "aifo:n:s:w:h",
		outObjFileStrDef[] = "-",
  		inObjFileStrDef[] = "-";

  opterr = 0;
  outObjFileStr = outObjFileStrDef;
  inObjFileStr = inObjFileStrDef;
  while(ok && ((option = getopt(argc, argv, optList)) != -1))
  {
    switch(option)
    {
      case 'a':
        asciiFlag = 1;
	break;
      case 'i':
        typeFlag = 1;
	binType = WLZ_HISTOGRAMDOMAIN_INT;
	break;
      case 'f':
        typeFlag = 1;
	binType = WLZ_HISTOGRAMDOMAIN_FLOAT;
	break;
      case 'o':
        outObjFileStr = optarg;
	break;
      case 'n':
	nBinsFlag = 1;
	if(sscanf(optarg, "%d", &nBins) != 1)
	{
	  usage = 1;
	  ok = 0;
	}
	break;
      case 's':
	originFlag = 1;
	if(sscanf(optarg, "%lg", &binOrigin) != 1)
	{
	  usage = 1;
	  ok = 0;
	}
	break;
      case 'w':
	widthFlag = 1;
	if(sscanf(optarg, "%lg", &binWidth) != 1)
	{
	  usage = 1;
	  ok = 0;
	}
	break;
      case 'h':
      default:
        usage = 1;
	ok = 0;
	break;
    }
  }
  if((inObjFileStr == NULL) || (*inObjFileStr == '\0') ||
     (outObjFileStr == NULL) || (*outObjFileStr == '\0'))
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

    histDom = inObj->domain.hist;
    binType = (typeFlag)? binType: histDom->type; 
    nBins = (nBinsFlag)? nBins: histDom->nBins; 
    binOrigin = (originFlag)? binOrigin: histDom->origin; 
    binWidth = (widthFlag)? binWidth: histDom->binSize; 
    if(((outObj = WlzAssignObject(WlzHistogramRebin(inObj, binType,
    						    nBins, nBins,
						    binOrigin, binWidth,
						   &errNum), NULL)) == NULL) ||
       (errNum != WLZ_ERR_NONE))
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
      		     "%s: failed to rebin histogram (%s)\n",
		     *argv, errMsg);
    }
  }
  if(ok)
  {
    if(asciiFlag)
    {
      if((fP = (strcmp(outObjFileStr, "-")?
               fopen(outObjFileStr, "w"): stdout)) == NULL)
      {
        ok = 0;
	(void )fprintf(stderr,
		       "%s: failed to write output data\n",
		       *argv);
      }
      else
      {
	histDom = outObj->domain.hist;
	switch(histDom->type)
	{
	  case WLZ_HISTOGRAMDOMAIN_INT:
	    for(idx = 0; idx < histDom->nBins; ++idx)
	    {
	      fprintf(fP, "%8g %8d\n",
		      histDom->origin + (idx * histDom->binSize),
		      *(histDom->binValues.inp + idx));
	    }
	    break;
	  case WLZ_HISTOGRAMDOMAIN_FLOAT:
	    for(idx = 0; idx < histDom->nBins; ++idx)
	    {
	      fprintf(fP, "%8g %8g\n",
		      histDom->origin + (idx * histDom->binSize),
		      *(histDom->binValues.dbp + idx));
	    }
	    break;
	}
      }
    }
    else
    {
      errNum = WLZ_ERR_WRITE_EOF;
      if(((fP = (strcmp(outObjFileStr, "-")?
		fopen(outObjFileStr, "w"):
		stdout)) == NULL) ||
	 ((errNum = WlzWriteObj(fP, outObj)) == 0))
      {
	ok = 0;
	(void )fprintf(stderr,
		       "%s: failed to write output object (%s).\n",
		       *argv, errMsg);
      }
    }
    if(fP && strcmp(outObjFileStr, "-"))
    {
      fclose(fP);
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
  if(usage)
  {
    (void )fprintf(stderr,
    "Usage: %s%sExample: %s%s",
    *argv,
    " [-h] [-a] [-o<out file>] [-i] [-f] [-n#] [-s#] [-w#] [<in object>]\n"
    "Options:\n"
    "  -a  Output the histogram as ascii data not a Woolz histogram object.\n"
    "  -o  Output object/data file name.\n"
    "  -i  Integer bin values.\n"
    "  -f  Floating point (double precision) bin values.\n"
    "  -n  Number of histogram bins.\n"
    "  -s  Start value, lower limit of histogram.\n"
    "  -w  Histogram bin width.\n"
    "  -h  Help, prints this usage message.\n"
    "Re-bins a Woolz histogram object.\n"
    "Objects/data are read from stdin and written to stdout unless the\n"
    "filenames are given.\n",
    *argv,
    " -s 0.0 -w 10.0 -n 30 -a myhist.wlz | xgraph\n"
    "The input Woolz histogram object is read from myhist.wlz, re-bined\n"
    "so that it starts at 0.0, has 30 bins of width 10.0, and is written\n",
    "as ascii to the program xgraph for display.\n");
  }
  return(!ok);
}
