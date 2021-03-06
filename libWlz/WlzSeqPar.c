#if defined(__GNUC__)
#ident "University of Edinburgh $Id$"
#else
static char _WlzSeqPar_c[] = "University of Edinburgh $Id$";
#endif
/*!
* \file         libWlz/WlzSeqPar.c
* \author       Jim Piper, Bill Hill
* \date         March 1999
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
* \brief	Performs sequential or local transformation of an object.
* 		Jim Piper derived the original seqpar.c from Fortran code by
* 		Marshall Presser.
* \ingroup	WlzValueFilters
*/

#include <Wlz.h>

/*!
* \return	Computed positive modulus.
* \ingroup	WlzValueFilters
* \brief	Computes the positive modulus of a given value.
* \param	val			Given value.
* \param	mod			Modulus to take.
*/
static int 	WlzSeqParPosMod(int val, int mod)
{
  if((val = val % mod) < 0)
  {
    val += mod;
  }
  return(val);
}

/*!
* \return	The filtered object, maybe NULL on error.
* \ingroup	WlzValueFilters
* \brief	WlzSeqPar performs a sequential or parallel local
*               transform. A distance transform is an example of a
*               sequential transform and a laplacian is an example of
*               a parallel transform.
*               Only WLZ_2D_DOMAINOBJ objects with values may be passed
*               to WlzSeqPar().
* \note		If the point to be transformed is at line l and col k,
*               there is an array of pointers spWSpace->adrptr[-7:7],
*               whose i'th entry gives the address of the point
*               (l + (i * spWSpace->ldelta), k), but which is only
*               meaningful for lines within bdrSz of the point.
*               For example: spWSpace->adrptr[-3] and
*               spWSpace->adrptr[3] are undefined if bdrSz < 3.
* \param	srcObj			The given WLZ_2D_DOMAINOBJ object.
* \param	newObjFlag		If zero then the given object is
*					overwritten otherwise a new object is
*					created.
* \param	sequentialFlag		If non zero the transform is
*                                       seqential and transformed
*                                       values are used in calculating
*                                       the neighbouring values.
*                                       If zero the transform always
*                                       works on the original grey
*                                       values.
* \param	rasterDir		Direction of raster scan.
* \param	bdrSz			Local transform kernel half-size,
*					must be in range 0 - 7.
*                                       The usual 8 immediate neighbors
*                                       correspond to bdrSz == 1.
* \param	bkgVal			Background grey value.
* \param	transformData		Data supplied to the transform
*                                       function.
* \param	transformFn		Supplied transform function.
* \param	dstErr			Destination error pointer, may
*                                       be NULL.
*/
WlzObject	*WlzSeqPar(WlzObject *srcObj,
			   int newObjFlag, int sequentialFlag,
			   WlzRasterDir rasterDir, int bdrSz, int bkgVal,
			   void *transformData,
			   int (*transformFn)(WlzSeqParWSpace *, void *),
			   WlzErrorNum *dstErr)
{
  int 		tI0,
  		curLine,
  		howManyLn,
		bdrP1,
		numBufs,
		kol,
		kol1,
		lineSz, 		/* lineSz is the maximally sized line,
					   including borders */
		lineOffset, 		/* lineOffset is difference in line
					   numbers between that being
					   transformed and that needed in the
					   input buffer for use in the
					   transform */
		nextLine,
		inLine,
		needed,
		idx,
		itop,
		minLine;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  int 		*lineBufSpace = NULL, 	/* lineBufSpace is the current line
  					   buffer */
  		*firstColP,
  		*lastColP,
		*dstBuf,
		*lastLnP,
		*tIP0;
  WlzIntervalDomain *jdp;
  WlzObject 	*dstObj = NULL;
  WlzSeqParWSpace spWSpace;		/* Work space which is passed on to
  					   the transform function. */
  WlzIntervalWSpace srcIWsp,
  		dstIWsp;
  WlzGreyWSpace srcGWsp,
  		dstGWsp;
  int		*lineBufs[15],      	/* lineBufs is the array of line
  					   buffers needed to obtain
					   neighbouring points. */
		*adrbase[15];

  WLZ_DBG((WLZ_DBG_LVL_FN|WLZ_DBG_LVL_1),
  	  ("WlzSeqPar FE %p %d %d %d %d %d %p %p %p\n",
	   srcObj, newObjFlag, sequentialFlag, (int )rasterDir, bdrSz, bkgVal,
	   transformData, transformFn, dstErr));
  if(srcObj == NULL)
  {
    errNum = WLZ_ERR_OBJECT_NULL;
    if(dstErr)
    {
      *dstErr = errNum;
    }
    return(NULL);
  }
  if((srcObj->type != WLZ_2D_DOMAINOBJ) || (srcObj->values.core == NULL) ||
     (srcObj->domain.core == NULL))
  {
    errNum = WLZ_ERR_OBJECT_TYPE;
    if(dstErr)
    {
      *dstErr = errNum;
    }
    return(NULL);
  }
  /*
   * Make an array spWSpace.adrptr of pointers with range [-7,7]
   */
  spWSpace.adrptr = adrbase + 7;
  spWSpace.brdrsz = bdrSz;
  jdp = srcObj->domain.i;
  bdrP1 = -bdrSz - 1;
  /*
   * The buffer must contain both the current line and bdrSz lines each side.
   */
  numBufs = (bdrSz * 2) + 1;
  kol1 = kol = jdp->kol1;
  lineSz = jdp->lastkl - kol + numBufs;
  /*
   * Some pointers to first/last line and column positions
   * to accomodate different rasters
   */
  firstColP = &dstIWsp.lftpos;
  lastColP = &dstIWsp.rgtpos;
  lastLnP = &jdp->lastln;
  /*
   * spWSpace.kdelta is increment to adrs when processing points
   * if positive, procede left to right in lines
   * if negative procede right to left
   * spWSpace.ldelta is increment to line number
   * if positive, the raster is proceeding in increasing line numbers
   * if negative the raster is procceding with decreasing line numbers
   */
  spWSpace.kdelta = 1;
  spWSpace.ldelta = 1;
  /*
   * Alter parameters for right to left rasters
   */
  if(!sequentialFlag)
  {
    rasterDir = WLZ_RASTERDIR_ILIC;
  }
  if((rasterDir == WLZ_RASTERDIR_ILDC) || (rasterDir == WLZ_RASTERDIR_DLDC))
  {
    tIP0 = firstColP;
    firstColP = lastColP;
    lastColP = tIP0;
    spWSpace.kdelta = -1;
  }
  /*
   * Alter parameters for bottom to top rasters
   */
  if((rasterDir == WLZ_RASTERDIR_DLIC) || (rasterDir == WLZ_RASTERDIR_DLDC))
  {
    spWSpace.ldelta = -1;
    lastLnP = &jdp->line1;
  }
  lineOffset = bdrSz * spWSpace.ldelta;
  /* 
   * Allocate space to buffers 
   */
  lineBufSpace = (int *)AlcCalloc(lineSz * (numBufs + 1), sizeof(int));
  for(idx = 0; idx < numBufs; ++idx)
  {
    lineBufs[idx] = lineBufSpace + (idx + 1) * lineSz;
  }
  /*
   * Either use existing object or create a new grey table object
   * according to the newObjFlag.
   * If a new object is required then it is created with the SAME interval
   * list and property list and a DIFFERENT but IDENTICAL grey table.
   */
  if(newObjFlag)
  {
    dstObj = WlzNewGrey(srcObj, &errNum);
  }
  else
  {
    dstObj = srcObj;
  }
  /*
   * Use nxxiv in transplant mode, placing and taking grey values
   * from the line buffers
   * srcIWsp is the workspace for entering values into the line buffers
   * dstIWsp is the workspace for processing the intervals and replacing
   * the transformed values into the grey table
   */
  srcGWsp.gvio = 1;		  /* output (since we are specifying tranpl) */
  dstGWsp.gvio = 0;						    /* input */
  errNum = WlzInitGreyRasterScan(srcObj, &srcIWsp, &srcGWsp, rasterDir, 1);
  if(errNum == WLZ_ERR_NONE)
  {
    errNum = WlzInitGreyRasterScan(dstObj, &dstIWsp, &dstGWsp, rasterDir, 1);
  }
  if(errNum != WLZ_ERR_NONE)
  {
    if(dstErr)
    {
      *dstErr = errNum;
    }
    return(NULL);
  }
  /*
   * curLine is the line currently being processed
   * inLine is the last line entered into the line buffers
   * initialize it to an imaginary line just before the first border line
   */
  (void )WlzNextInterval(&srcIWsp);
  nextLine = srcIWsp.linpos;
  inLine = srcIWsp.linpos - (spWSpace.ldelta * (bdrSz + 1));
  /*
   * Process the next interval
   */
  while((errNum == WLZ_ERR_NONE) &&
        ((errNum = WlzNextGreyInterval(&dstIWsp)) == WLZ_ERR_NONE))
  {
    /*
     * When processing curLine, the line buffer must contain
     * lines upto needed=curLine+border width
     */
    curLine = dstIWsp.linpos;
    needed = curLine + lineOffset;
    /*
     * Next action depends which lines are aleady in the line buffers
     */
    howManyLn = (needed - inLine) * spWSpace.ldelta;
    WLZ_DBG((WLZ_DBG_LVL_3),
  	    ("WlzSeqPar 01 %d %d\n",
	     inLine, curLine));
    if(howManyLn < 0)
    {
      /*
       * This is an error it is impossible to have input a line before needed
       */
      errNum = WLZ_ERR_DOMAIN_DATA;
      if(dstErr)
      {
        *dstErr = errNum;
      }
      return(NULL);
    }
    else if(howManyLn > 0)
    {
      /*
       * If howManyLn > 0 we need to input some lines to input buffer
       * routine to fill the input buffers used to determine the value
       * of the transform needed = maximal(minimal)line needed to be filled
       * into the buffer
       * if backwards raster, needed = curLine - bdrSz
       * if forwards raster, needed = curLine + bdrSz
       * nextLine = line number of the next interval to be filed by nxxiv
       */
      while(1)
      {
	if((needed - nextLine) * spWSpace.ldelta < 0)
	{
	  /*
	   * Next line needed less far away than next line available
	   * from nxxint fill required buffer lines with bkgVal and return
	   * minLine = WlzSeqParPosMod(inLine, numBufs), i.e. the remainder
	   * of inLine when divided by numBufs.
	   * It thus can be used as the offset into the array of
	   * logical line addresses to determine which line in the
	   * circular buffer inLine is being stored
	   */
	  while(inLine != needed)
	  {
	    inLine += spWSpace.ldelta;
	    minLine = WlzSeqParPosMod(inLine, numBufs);
	    WlzValueSetInt(lineBufs[minLine], bkgVal, lineSz);
	  }
	  break;
	}
	/*
	 * Line(s) needed include some real lines, i.e. obtained from nxxiv
	 * first fill buffer with bkgVal
	 */
	while(inLine != nextLine)
	{
	  inLine += spWSpace.ldelta;
	  minLine = WlzSeqParPosMod(inLine, numBufs);
	  WlzValueSetInt(lineBufs[minLine], bkgVal, lineSz);
	}
	/*
	 * nxxint interval to be put into buffer calculate adrs in
	 * buffer by adding leftcol to logical adrs stored in
	 * lineBufs[minLine]. then insert with nxxiv
	 */
	do
	{
	  srcGWsp.u_grintptr.inp = lineBufs[minLine] +
	  			   srcIWsp.lftpos + bdrSz - kol1;
	  WlzGreyInterval(&srcIWsp);
	  /*
	   * Call nxxint to get next interval if in same line store in buffer
	   */
	  if(WlzNextInterval(&srcIWsp) != 0)
	  {
	    /*
	     * When nxxint has finished all interval set a high value into
	     * nextLine, so all future lines input will be dummy lines
	     */
	    nextLine = (*lastLnP) - (bdrP1 * spWSpace.ldelta);
	    goto filledLABEL;
	  }
	} while(srcIWsp.nwlpos == 0);
	/*
	 * If next interval in new line,store new line number and
	 * jump to start of routine to see what sort of line buffer
	 * processing is required next
	 */
	nextLine = srcIWsp.linpos;
      }
    }
    /*
     * Line buffers filled for processing of this line either from above call
     * or buffer filled for previous intervals processed on this line
     * kol is the first point in the interval to be processed
     * itop is the last point to be processed
     */
filledLABEL: 					    /* LABEL! see goto above */
    kol = *firstColP;
    itop = *lastColP;
    /*
     * First load spWSpace.adrptr with addresses of required neighbors
     */
    idx = bdrP1;
    while(idx++ < bdrSz)
    {
      tI0 = idx * spWSpace.ldelta;
      spWSpace.adrptr[idx] = lineBufs[WlzSeqParPosMod(curLine + tI0,
      						      numBufs)] +
			     kol + bdrSz - kol1;
    }
    /*
     * Then set the pointer where the transformed value will be placed
     * if parallel mode, adrs is in current line buffer
     * if sequential mode, adrs is in input buffer
     * dstGWsp.grintptr is the location in the nxxiv workspace
     * giving the location from which nxxiv will transplant
     * values into the grey table
     */
    if(sequentialFlag)
    {
      dstBuf = spWSpace.adrptr[0];
    }
    else
    {
      dstBuf = lineBufSpace + bdrSz;
    }
    dstGWsp.u_grintptr.inp = dstBuf + dstIWsp.lftpos - kol;
    /*
     * Procede thru interval,calling the transforming function
     * at each point and storing the result where required
     */
    kol -= spWSpace.kdelta;
    while(kol != itop)
    {
      kol += spWSpace.kdelta;
      *dstBuf = (*transformFn)(&spWSpace, transformData);
      /*
       * after each point, update the neighbor adrs and the output adrs
       */
      idx = bdrP1;
      while(idx++ < bdrSz)
      {
	spWSpace.adrptr[idx] += spWSpace.kdelta;
      }
      dstBuf += spWSpace.kdelta;
    }
    /*
     * Having finished this interval proceed to the next
     */
  }
  if(errNum == WLZ_ERR_EOO)
  {
    errNum = WLZ_ERR_NONE;
  }
  if(lineBufSpace)
  {
    AlcFree((void *)lineBufSpace);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  WLZ_DBG((WLZ_DBG_LVL_FN|WLZ_DBG_LVL_1),
  	  ("WlzSeqPar FX %p\n",
	   dstObj));
  return(dstObj);
}
