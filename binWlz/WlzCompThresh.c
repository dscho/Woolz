#pragma ident "MRC HGU $Id$"
/***********************************************************************
* Project:      Woolz
* Title:        WlzCompThresh.c
* Date:         March 1999
* Author:       Bill Hill
* Copyright:	1999 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:      Computes a threshold value from a Woolz histogram
*		object.
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
  int		option,
		ok = 1,
		usage = 0;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  FILE		*fP = NULL;
  WlzObject	*inObj = NULL;
  WlzCompThreshType thrMethod = WLZ_COMPTHRESH_GRADIENT;
  double	extFrac = 0.0,
  		thrVal;
  char 		*outFileStr,
  		*inObjFileStr;
  const char	*errMsg;
  static char	optList[] = "o:x:dfgh",
		outFileStrDef[] = "-",
  		inObjFileStrDef[] = "-";

  opterr = 0;
  outFileStr = outFileStrDef;
  inObjFileStr = inObjFileStrDef;
  while(ok && ((option = getopt(argc, argv, optList)) != -1))
  {
    switch(option)
    {
      case 'o':
        outFileStr = optarg;
	break;
      case 'd':
        thrMethod = WLZ_COMPTHRESH_DEPTH;
	break;
      case 'f':
        thrMethod = WLZ_COMPTHRESH_FOOT;
	break;
      case 'g':
        thrMethod = WLZ_COMPTHRESH_GRADIENT;
	break;
      case 'x':
        if(sscanf(optarg, "%lg", &extFrac) != 1)
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
     (outFileStr == NULL) || (*outFileStr == '\0'))
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
       ((inObj= WlzReadObj(fP, &errNum)) == NULL))
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
		     "%s: failed to read object from file %s (%s)\n",
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
      		     "%s: input object is not a valid histogram object\n",
		     *argv);
    }
  }
  if(ok)
  {
    if((errNum = WlzCompThreshold(&thrVal, inObj, thrMethod,
    				  extFrac)) != WLZ_ERR_NONE)
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
      		     "%s: failed to compute threshold value (%s).\n",
		     *argv, errMsg);
    }
  }
  if(ok)
  {
    if(((fP = (strcmp(outFileStr, "-")?
              fopen(outFileStr, "w"):
	      stdout)) == NULL))
    {
      ok = 0;
      (void )fprintf(stderr,
      		     "%s: failed to write output object\n",
		     *argv);
    }
    else
    {
      (void )fprintf(fP, "%g\n", thrVal);
    }
    if(fP && strcmp(outFileStr, "-"))
    {
      fclose(fP);
    }
  }
  if(usage)
  {
    (void )fprintf(stderr,
    "Usage: %s%sExample: %s%s",
    *argv,
    " [-o<out file>] [-d] [-f] [-g] [-x#] [-h] [<in object>]\n"
    "Options:\n"
    "  -o  Output file name.\n"
    "  -d  Use histogram depth method.\n"
    "  -f  Use histogram foot method.\n"
    "  -g  Use histogram gradient method.\n"
    "  -h  Help, prints this usage message.\n"
    "  -x# Extra fraction to add to computed threshold value.\n"
    "Computes a threshold value from the given histogram using one of the\n"
    "following methods:\n"
    "  WLZ_COMPTHRESH_DEPTH The threshold value is the point on the\n"
    "    histogram which is maximally (perpendicularly) distant from the\n"
    "    chord joining the histogram peak with the right hand histogram end\n"
    "    point.\n"
    "  WLZ_COMPTHRESH_FOOT The threshold value is intercept of a line fitted\n"
    "    to the upper slope (90% to 33%) to the right of the histogram main\n"
    "    peak with the abscissa.\n"
    "  WLZ_COMPTHRESH_GRADIENT The threshold value is the first point to the\n"
    "    right of the histogram main peak at which the gradient falls to\n"
    "    zero.\n"
    "The input object is read from stdin and values are written to stdout\n"
    "unless the filenames are given.\n",
    *argv,
    " -g myhist.wlz\n"
    "The threshold value is computed using the WLZ_COMPTHRESH_GRADIENT\n"
    "method + 5%% and written to the standard output.\n");
  }
  return(!ok);
}
