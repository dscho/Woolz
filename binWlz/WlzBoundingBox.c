#pragma ident "MRC HGU $Id$"
/*!
\ingroup      BinWlz
\defgroup     wlzboundingbox WlzBoundingBox
\par Name
WlzBoundingBox - Calculates the bounding box of the given object.
\par Synopsis
\verbatim
WlzBoundingBox [-o <output file>] [-h] [<input object file>]

\endverbatim
\par Options
<table width="500" border="0">
  <tr>
    <td><b>-o</b></td>
    <td>output file name</td>
  </tr>
  <tr>
    <td><b>-h</b></td>
    <td>Help - print help message</td>
  </tr>
</table>
By default the input object is read from the standard input
and the output is written to the standard output.

\par Description
WlzBoundingBox calculates the bounding box
of the given Woolz object.
The bounding box is written in the following order:
\<x min\> \<y min\> \<z min\> \<x max\> \<y max\> \<z max\>

\par Examples
\verbatim
# WlzBoundingBox reads an object from myobj.wlz, calculates
# its bounding box and then prints it to the standard output.

WlzBoundingBox myobj.wlz
\endverbatim
\par See Also
WlzBoundingBox(3)
\par Bugs
None known
\author       richard <Richard.Baldock@hgu.mrc.ac.uk>
\date         Wed Jul 27 08:13:41 2005
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
* Title:        WlzBoundingBox.c
* Date:         March 1999
* Author:       Bill Hill
* Copyright:	1999 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:      Computes the bounding box of a 2 or 3D woolz object
*		and prints the output in the format:
*		  <x min> <y min> <z min> <x max> <y max> <z max>
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
  WlzDBox3	bBox;
  char 		*outFileStr,
  		*inObjFileStr;
  static char	optList[] = "o:h",
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
    if((inObjFileStr == NULL) ||
       (*inObjFileStr == '\0') ||
       ((fP = (strcmp(inObjFileStr, "-")?
	      fopen(inObjFileStr, "r"): stdin)) == NULL) ||
       ((inObj= WlzReadObj(fP, &errNum)) == NULL) ||
       (errNum != WLZ_ERR_NONE))
    {
      ok = 0;
      (void )fprintf(stderr,
		     "%s: failed to read object from file %s\n",
		     *argv, inObjFileStr);
    }
    if(fP && strcmp(inObjFileStr, "-"))
    {
      fclose(fP);
    }
  }
  if(ok)
  {
    bBox = WlzBoundingBox3D(inObj, &errNum);
    if(errNum != WLZ_ERR_NONE)
    {
      ok = 0;
      (void )fprintf(stderr,
      		     "%s: failed to compute bounding box of object\n",
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
      		     "%s: failed to write output data\n",
		     *argv);
    }
    else
    {
      (void )fprintf(fP, "%g %g %g %g %g %g\n",
      		     bBox.xMin, bBox.yMin, bBox.zMin,
		     bBox.xMax, bBox.yMax, bBox.zMax);
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
    " [-o<out file>] [-h] [<in object>]\n"
    "Options:\n"
    "  -o  Output file name.\n"
    "  -h  Help, prints this usage message.\n"
    "Calculates the bounding box of the input Woolz object. The data\n"
    "are written to the output file in the following order:\n"
    "  <x min> <y min> <z min> <x max> <y max> <z max>\n"
    "The input object is read from stdin and data are written to stdout\n"
    "unless the filenames are given.\n",
    *argv,
    " -o out.txt myobj.wlz\n"
    "The input Woolz object is read from myobj.wlz. It's bounding box\n"
    "is computed and written to out.txt.\n");
  }
  return(!ok);
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
