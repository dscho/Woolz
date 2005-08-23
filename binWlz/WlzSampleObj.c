#pragma ident "MRC HGU $Id$"
/*!
\ingroup      BinWlz
\defgroup     wlzsampleobj WlzSampleObj
\par Name
WlzSampleObj - Sub-samples Woolz 2D domain objects objects.
\par Synopsis
\verbatim
WlzSampleObj [-o<output object file>]
           [-x<column sampling factor>]
           [-y<line sampling factor>]
           [-z<plane sampling factor>]
           [-a] [-e] [-i] [-g] [-m] [-p] [-h] [<input object file>]


\endverbatim
\par Options
<table width="500" border="0">
  <tr>
    <td><b>-x</b></td>
    <td>column sub-sampling factor. </td>
  </tr>
  <tr>
    <td><b>-y</b></td>
    <td>line sub-sampling factor. </td>
  </tr>
  <tr>
    <td><b>-z</b></td>
    <td>plane sub-sampling factor. </td>
  </tr>
  <tr>
    <td><b>-a</b></td>
    <td>maximum value convolution kernel. </td>
  </tr>
  <tr>
    <td><b>-e</b></td>
    <td>median value convolution kernel. </td>
  </tr>
  <tr>
    <td><b>-i</b></td>
    <td>minimum value convolution kernel. </td>
  </tr>
  <tr>
    <td><b>-g</b></td>
    <td>gaussian convolution kernel. </td>
  </tr>
  <tr>
    <td><b>-m</b></td>
    <td>mean convolution kernel. </td>
  </tr>
  <tr>
    <td><b>-p</b></td>
    <td> point sampling.</td>
  </tr>
  <tr>
    <td><b>-o</b></td>
    <td>Output object file name.</td>
  </tr>
  <tr>
    <td><b>-h</b></td>
    <td>Help - print help message</td>
  </tr>
  <tr>
    <td><b>-v</b></td>
    <td>Verbose operation</td>
  </tr>
</table>
By  default  the  input  object is read from the standard input and the
output object is written to the standard output.
\par Description
Sub-samples a woolz interval domain object with grey values.

\par Examples
\verbatim
# An example of using WlzSampleObj to apply a scale transform
# (scale = 0.5) using point sampling:

WlzSampleObj -x 2 -y 2 -p <infile.wlz > outfile.wlz

# Sub-sampling an object using a gaussian convolution kernel
# with scale along rows = 0.3333 and the scale down columns
# 0.5.

WlzSobel -x 3 -y 2 -g -o outfile.wlz infile.wlz

\endverbatim

\par See Also
WlzSampleObj(3).
\par Bugs
None known
\author       richard <Richard.Baldock@hgu.mrc.ac.uk>
\date         Fri Jul 29 11:57:19 2005
\version      MRC HGU $Id$
              $Revision$
              $Name$
\par Copyright:
             1994-2003 Medical Research Council, UK.
              All rights reserved.
\par Address:
              MRC Human Genetics Unit,
              Western General Hospital,
              Edinburgh, EH4 2XU, UK.
*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/***********************************************************************
* Project:      Woolz
* Title:        WlzSampleObj.c
* Date:         March 1999
* Author:       Bill Hill
* Copyright:	1999 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:      Woolz filter which applies a sub-sampling filter to
*		the given object.
* $Revision$
* Maintenance:	Log changes below, with most recent at top of list.
* 22-09-00 bill Make changes fro 3D sampling and free objects to allow
*		checking for memory leaks.
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
  WlzIVertex3	samFac;
  WlzSampleFn	samFn;
  char		*outObjFileStr,
  		*inObjFileStr;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  FILE		*fP = NULL;
  WlzObject	*inObj = NULL,
  		*outObj = NULL;
  const char	*errMsg;
  static char	optList[] = "o:x:y:z:aegipmh",
		outObjFileStrDef[] = "-",
  		inObjFileStrDef[] = "-";

  samFac.vtX = 1;
  samFac.vtY = 1;
  samFac.vtZ = 1;
  samFn = WLZ_SAMPLEFN_POINT;
  opterr = 0;
  inObjFileStr = inObjFileStrDef;
  outObjFileStr = outObjFileStrDef;
  while(ok && (usage == 0) &&
  	((option = getopt(argc, argv, optList)) != -1))
  {
    switch(option)
    {
      case 'a':
	samFn = WLZ_SAMPLEFN_MAX;
	break;
      case 'e':
	samFn = WLZ_SAMPLEFN_MEDIAN;
	break;
      case 'i':
	samFn = WLZ_SAMPLEFN_MIN;
	break;
      case 'o':
        outObjFileStr = optarg;
	break;
      case 'g':
	samFn = WLZ_SAMPLEFN_GAUSS;
	break;
      case 'm':
	samFn = WLZ_SAMPLEFN_MEAN;
	break;
      case 'p':
	samFn = WLZ_SAMPLEFN_POINT;
	break;
      case 'x':
        if(sscanf(optarg, "%d", &(samFac.vtX)) != 1)
	{
	  usage = 1;
	  ok = 0;
	}
	break;
      case 'y':
        if(sscanf(optarg, "%d", &(samFac.vtY)) != 1)
	{
	  usage = 1;
	  ok = 0;
	}
	break;
      case 'z':
        if(sscanf(optarg, "%d", &(samFac.vtZ)) != 1)
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
    if((outObj = WlzSampleObj(inObj, samFac, samFn, &errNum)) == NULL)
    {
    ok = 0;
    (void )WlzStringFromErrorNum(errNum, &errMsg);
    (void )fprintf(stderr,
    		   "%s: failed to sub-sample object (%s).\n",
		   *argv, errMsg);
    }
  }
  if(ok)
  {
    errNum = WLZ_ERR_WRITE_EOF;
    if(((fP = (strcmp(outObjFileStr, "-")?
              fopen(outObjFileStr, "w"):
              stdout)) == NULL) ||
       ((errNum = WlzWriteObj(fP, outObj)) != WLZ_ERR_NONE))
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
                     "%s: failed to write output object (%s).\n",
                     *argv, errMsg);
    }
    if(fP && strcmp(outObjFileStr, "-"))
    {
      fclose(fP);
    }
  }
  if(inObj)
  {
    (void )WlzFreeObj(inObj);
  }
  if(outObj)
  {
    (void )WlzFreeObj(outObj);
  }
  if(usage)
  {
    (void )fprintf(stderr,
    "Usage: "
    "%s [-x#] [-y#] [-z#] [-k#] [-l#] [-g] [-m] [-p] [-h] [<in object>]\n"
    "Sub-samples a woolz interval domain object with grey values.\n"
    "Options are:\n"
    "  -x#   Sampling factor for rows (set to %d).\n"
    "  -y#   Sampling factor for columns (set to %d).\n"
    "  -z#   Sampling factor for planes (set to %d).\n"
    "  -o#   Output object file name.\n"
    "  -a    Use max sampling kernel (%sset).\n"
    "  -e    Use median sampling kernel (%sset).\n"
    "  -i    Use min sampling kernel (%sset).\n"
    "  -g    Use gaussian sampling kernel (%sset).\n"
    "  -m    Use mean sampling kernel (%sset).\n"
    "  -p    Use point sampling (%sset).\n"
    "  -h    Display this usage information.\n",
    *argv,
    samFac.vtX, samFac.vtY, samFac.vtZ,
    (samFn == WLZ_SAMPLEFN_MAX) ? "" : "not ",
    (samFn == WLZ_SAMPLEFN_MEDIAN) ? "" : "not ",
    (samFn == WLZ_SAMPLEFN_MIN) ? "" : "not ",
    (samFn == WLZ_SAMPLEFN_GAUSS) ? "" : "not ",
    (samFn == WLZ_SAMPLEFN_MEAN) ? "" : "not ",
    (samFn == WLZ_SAMPLEFN_POINT) ? "" : "not ");
  }
  return(!ok);
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
