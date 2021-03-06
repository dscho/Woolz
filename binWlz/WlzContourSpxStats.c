#if defined(__GNUC__)
#ident "University of Edinburgh $Id$"
#else
static char _WlzContourSpxStats_c[] = "University of Edinburgh $Id$";
#endif
/*!
* \file         binWlz/WlzContourSpxStats.c
* \author       Bill Hill
* \date         December 2003
* \version      $Id$
* \par
* Address:
*               MRC Human Genetics Unit,
*               MRC Institute of Genetics and Molecular Medicine,
*               University of Edinburgh,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \par
* Copyright (C), [2012],
* The University Court of the University of Edinburgh,
* Old College, Edinburgh, UK.
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be
* useful but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
* \brief	Calculates simplex statistics for contour models.
* \ingroup	BinWlz
*
* \par Binary
* \ref wlzcontourspxstats "WlzContourSpxStats"
*/

/*!
\ingroup BinWlz
\defgroup wlzcontourspxstats WlzContourSpxStats
\par Name
WlzContourSpxStats - calculates simplex statistics for contour models.
\par Synopsis
\verbatim
WlzContourSpxStats [-o<out file>] [-v] [-h] [<in object>]
\endverbatim
\par Options
<table width="500" border="0">
  <tr> 
    <td><b>-h</b></td>
    <td>Help, prints usage message.</td>
  </tr>
  <tr> 
    <td><b>-o</b></td>
    <td>Output file name.</td>
  </tr>
  <tr> 
    <td><b>-v</b></td>
    <td>Verbose output flag.</td>
  </tr>
</table>
\par Description
Calculates statistics of the simplices in the given objects contour.
These are: The number of simplices together with the minimum, maximum,
sum, sum of squares, mean and the standard deviation of either the
length (for 2D contours) of the area (for 3D contours) of the input
contour's simplices.
If the verbose output flag is not set then the following are written
to the output file in the following order:
\verbatim
  <area> <min> <max> <sum> <sum of sq> <mean> <std dev>.
\endverbatim
The input object is read from stdin and values are written to stdout
unless the filenames are given.

\par Examples
\verbatim
WlzContourSpxStats -o stats.txt myctr.wlz
\endverbatim
The input Woolz object is read from myctr.wlz. The statistics are
calculated and  written to out.txt.
\par File
\ref WlzContourSpxStats.c "WlzContourSpxStats.c"
\par See Also
\ref BinWlz "WlzIntro(1)"
\ref wlzcontourobj "WlzContourObj(1)"
\ref WlzGMModelSpxStats "WlzGMModelSpxStats(3)"
*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
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
  int		nNMEdge = -1,
  		nSpx,
  		option,
		ok = 1,
		verbose = 0,
		usage = 0;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  FILE		*fP = NULL;
  WlzObject	*inObj = NULL;
  WlzGMEdge	**edgeTP;
  WlzGMModel	*model;
  double	min,
  		max,
		sum,
		sumSq,
		mean,
		stdDev;
  char 		*outFileStr,
  		*inObjFileStr;
  const char	*errMsg;
  static char	optList[] = "o:vh",
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
      case 'v':
        verbose = 1;
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
       ((inObj = WlzReadObj(fP, &errNum)) == NULL))
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
		     "%s: failed to read object from file %s (%s).\n",
		     *argv, inObjFileStr,
		     errMsg);
    }
    if(fP && strcmp(inObjFileStr, "-"))
    {
      fclose(fP);
    }
  }
  if(ok)
  {
    if(inObj->type != WLZ_CONTOUR)
    {
      errNum = WLZ_ERR_OBJECT_TYPE;
    }
    else if((inObj->domain.ctr == NULL) ||
            ((model = inObj->domain.ctr->model) == NULL))
    {
      errNum = WLZ_ERR_DOMAIN_NULL;
    }
    if(errNum != WLZ_ERR_NONE)
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
                     "%s: Invalid input object (%s).\n",
		     *argv, errMsg);
    }
  }
  if(ok)
  {
    switch(model->type)
    {
      case WLZ_GMMOD_2I:
      case WLZ_GMMOD_2D:
      case WLZ_GMMOD_2N:
	break;
      case WLZ_GMMOD_3I:
      case WLZ_GMMOD_3D:
      case WLZ_GMMOD_3N:
	if((edgeTP = WlzGMModelFindNMEdges(model, &nNMEdge, &errNum)) != NULL)
	{
	  (void )AlcFree(edgeTP);
	}
	break;
      default:
	errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
    if(ok)
    {
      nSpx = WlzGMModelSpxStats(model,
				&min, &max, &sum, &sumSq, &mean, &stdDev,
				&errNum);
    }
    if(errNum != WLZ_ERR_NONE)
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
		     "%s: failed to compute statistics (%s)\n",
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
      		     "%s: failed to write output file\n",
		     *argv);
    }
    else
    {
      if(verbose)
      {
        (void )fprintf(fP,
		       "n spx       % 14d\n"
		       "min         % 14g\n"
		       "max         % 14g\n"
		       "sum         % 14g\n"
		       "sum sq      % 14g\n"
		       "mean        % 14g\n"
		       "std dev     % 14g\n",
		       nSpx, min, max, sum, sumSq, mean, stdDev);
        if(nNMEdge >= 0)
	{
	  (void )fprintf(fP,
			 "n NM edge   % 14d\n",
			 nNMEdge);
	}
      }
      else
      {
      (void )fprintf(fP, "%d %g %g %g %g %g %g\n",
      		     nSpx, min, max, sum, sumSq, mean, stdDev);
      }
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
    " [-o<out file>] [-v] [-h] [<in object>]\n"
    "Options:\n"
    "  -o  Output file name.\n"
    "  -v  Verbose output flag.\n"
    "  -h  Help, prints this usage message.\n"
    "Calculates statistics of the simplices in the given objects contour.\n"
    "These are: The number of simplices together with the minimum, maximum,\n"
    "sum, sum of squares, mean and the standard deviation of either the\n"
    "length (for 2D contours) of the area (for 3D contours) of the input\n"
    "contour's simplices.\n"
    "If the verbose output flag is not set then the following are written\n"
    "to the output file in the following order:\n"
    "  <area> <min> <max> <sum> <sum of sq> <mean> <std dev>.\n"
    "The input object is read from stdin and values are written to stdout\n"
    "unless the filenames are given.\n",
    *argv,
    " -o stats.txt myctr.wlz\n"
    "The input Woolz object is read from myctr.wlz. The statistics are\n"
    "calculated and  written to out.txt.\n");
  }
  return(!ok);
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
