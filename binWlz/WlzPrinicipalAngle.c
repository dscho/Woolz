#pragma ident "MRC HGU $Id$"
/***********************************************************************
* Project:      Woolz
* Title:        WlzPrincipalAngle.c
* Date:         March 1999
* Author:       Bill Hill
* Copyright:	1999 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:	Calculates the mass, centre of mass and principal angle
*		of the input Woolz 2D domain object. The mass, centre of
*		mass and principal angle are written to the output file
*		using the format:
*		  <mass> <c of m x> <c of m y> <angle>
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
		binObjFlag = 0,
		degreesFlag = 0,
		ok = 1,
		usage = 0;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  FILE		*fP = NULL;
  WlzObject	*inObj = NULL;
  double	pAngle,
  		mass;
  WlzDVertex2	cOfMass;
  char 		*outFileStr,
  		*inObjFileStr;
  const char	*errMsg;
  static char	optList[] = "o:bdh",
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
      case 'b':
        binObjFlag = 1;
	break;
      case 'd':
        degreesFlag = 1;
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
    cOfMass = WlzCentreOfMass2D(inObj, binObjFlag, &mass, &errNum);
    if(errNum != WLZ_ERR_NONE)
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
      		     "%s: failed to compute centre of mass of object (%s).\n",
		     *argv, errMsg);
    }
  }
  if(ok)
  {
    pAngle = WlzPrincipalAngle(inObj, cOfMass, binObjFlag, &errNum);
    if(errNum != WLZ_ERR_NONE)
    {
      ok = 0;
      (void )fprintf(stderr,
      		     "%s: failed to compute principal angle of object\n",
		     *argv);
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
      if(degreesFlag)
      {
        pAngle *= 180 / WLZ_M_PI;
      }
      (void )fprintf(fP, "%g %g %g %g\n",
      		     mass, cOfMass.vtX, cOfMass.vtY, pAngle);
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
    " [-o<out file>] [-b] [-d] [-h] [<in object>]\n"
    "Options:\n"
    "  -o  Output file name.\n"
    "  -b  Always consider object a binary object.\n"
    "  -d  Output angle in degrees instead of radians.\n"
    "  -h  Help, prints this usage message.\n"
    "Calculates the mass, centre of mass and principal angle of the input\n"
    "Woolz 2D domain object. The principal angle is the angle which the long\n"
    "principal axis makes with the x-axis in the given object.\n"
    "The mass, centre of mass and principal angle are written to the output\n"
    "file in the following order:\n"
    "  <mass> <x> <y> <angle>\n"
    "Where x and y are the column and line coordinates of the centre of\n"
    "mass.\n"
    "The input object is read from stdin and values are written to stdout\n"
    "unless the filenames are given.\n",
    *argv,
    " -o out.txt myobj.wlz\n"
    "The input Woolz object is read from myobj.wlz. It's mass, centre of\n"
    "mass and principal angle are written to out.txt.\n");
  }
  return(!ok);
}
