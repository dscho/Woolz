#pragma ident "MRC HGU $Id$"
/************************************************************************
* Project:	Mouse Atlas
* Title:        ReconstructSection.c				
* Date:         April 1999
* Author:       Bill Hill                                              
* Copyright:    1999 Medical Research Council, UK.
*		All rights reserved.				
* Address:	MRC Human Genetics Unit,			
*		Western General Hospital,			
*		Edinburgh, EH4 2XU, UK.				
* Purpose:      Provides functions for manipulationg sections and
*		section lists for the MRC Human Genetics Unit	
*		reconstruction library.				
* $Revision$
* Maintenance:  Log changes below, with most recent at top of list.    
************************************************************************/
#include <Reconstruct.h>
#include <string.h>

typedef struct
{
  int		indexCur;
  int		indexInc;
} RecSecListIndexArgs;

typedef struct
{
  unsigned int	checkMask;
  unsigned int	invalidMask;
  RecSection	*prevSec;
} RecSecListInvalidArgs;

static int	RecSecSortFnSignCmpDbl(double, double),
		RecSecSortFnTransf(void *, void *, unsigned int),
		RecSecSortFnIndex(void *, void *),
		RecSecSortFnIterations(void *, void *),
		RecSecSortFnCorrel(void *, void *),
		RecSecSortFnImageFile(void *, void *),
		RecSecSortFnTransfTx(void *, void *),
		RecSecSortFnTransfTy(void *, void *),
		RecSecSortFnTransfTz(void *, void *),
		RecSecSortFnTransfScale(void *, void *),
		RecSecSortFnTransfTheta(void *, void *),
		RecSecSortFnTransfPhi(void *, void *),
		RecSecSortFnTransfAlpha(void *, void *),
		RecSecSortFnTransfPsi(void *, void *),
		RecSecSortFnTransfXsi(void *, void *),
		RecSecSortFnTransfInvert(void *, void *),
		RecSecListIndexFn(HGUDlpList *, HGUDlpListItem *, void *),
		RecSecListInvalidFn(HGUDlpList *, HGUDlpListItem *, void *),
		RecSecCumTransfClearItFn(HGUDlpList *, HGUDlpListItem *,
					 void *),
		RecSecCumTransfSetItFn(HGUDlpList *, HGUDlpListItem *, void *),
		RecSecFindItemIndexItFn(HGUDlpList *, HGUDlpListItem *,
					void *);

/************************************************************************
* Function:	RecSecAssign					
* Returns:	RecSection *:		Section with incremented link-
*					count, if non NULL.	
* Purpose:	Increments the given sections link count to ease 
*		registration section assignment.		
* Global refs:	-						
* Parameters:	RecSection *sec:	Section to be assigned.	
************************************************************************/
RecSection	*RecSecAssign(RecSection *sec)
{
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecAssign FE 0x%lx\n",
	   (unsigned long )sec));
  if(sec)
  {
    ++(sec->linkcount);
    REC_DBG((REC_DBG_SEC|REC_DBG_LVL_3),
	    ("RecSecAssign 01 %d\n",
	     sec->linkcount));
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecAssign FX 0x%lx\n",
	   (unsigned long )sec));
  return(sec);
}

/************************************************************************
* Function:	RecSecDup					
* Returns:	RecSection *:		Duplicated section maybe NULL
*					on error.		
* Purpose:	Duplicates the given section
* Global refs:	-						
* Parameters:	RecSection *sec:	Section to be assigned.	
************************************************************************/
RecSection	*RecSecDup(RecSection *sec)
{
  RecSection	*newSec = NULL;
  WlzErrorNum	wlzErr = WLZ_ERR_NONE;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecDup FE 0x%lx\n",
	   (unsigned long )sec));
  if(sec)
  {
    if(((newSec = (RecSection *)AlcCalloc(1, sizeof(RecSection))) != NULL) &&
       sec->imageFile &&
       ((newSec->imageFile = AlcStrDup(sec->imageFile)) != NULL) &&
       sec->transform &&
       ((newSec->transform = WlzAssignAffineTransform(
			 WlzAffineTransformFromMatrix4X4(sec->transform->type,
       					      sec->transform->mat, &wlzErr),
					      NULL)) != NULL) &&
       (wlzErr == WLZ_ERR_NONE))
    {
      newSec->index = sec->index;
      newSec->iterations = sec->iterations;
      newSec->correl = sec->correl;
    }
    else
    {
      if(newSec)
      {
        if(newSec->imageFile)
	{
	  AlcFree(newSec->imageFile);
	}
        AlcFree(newSec);
        newSec = NULL;
      }
    }
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecDup FX 0x%lx\n",
	   (unsigned long )newSec));
  return(newSec);
}

/************************************************************************
* Function:	RecSecMake					
* Returns:	RecSection *:		New registration section.
* Purpose:	Makes a registration section using the given member
*		values.						
* Global refs:	-						
* Parameters:	int index:		Section index.		
*		int iterations:		Number of iterations to find
*					sections transform.	
*		double correlation:	Sections correlation value.
*		char *imageFile:	Image file path, this is 
*					duplicated so that the original
*					may be AlcFree'd. The image file
*					path may not be NULL.	
*		WlzAffineTransform *transform: Section transform, if NULL an
*					identity transform is created.
*		WlzObject *obj:		Woolz object corresponding to
*					the given image file. This may
*					be NULL without causing the
*					object to be read from the
*					associated file.	
************************************************************************/
RecSection	*RecSecMake(int index, int iterations, double correlation,
			    char *imageFile,
			    WlzAffineTransform *transform, WlzObject *obj)
{
  RecSection	*sec = NULL;
  char		*newImageFile = NULL;
  WlzAffineTransform	*newTransform = NULL;
  WlzErrorNum	wlzErr = WLZ_ERR_NONE;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecMake FE %d %d %g 0x%lx 0x%lx 0x%lx\n",
	   index, iterations, correlation,
	   (unsigned long )imageFile, (unsigned long )transform,
	   (unsigned long )obj));
  if(imageFile)
  {
    newImageFile = AlcStrDup(imageFile);
  }
  if(newImageFile && (transform == NULL))
  {
    newTransform = WlzAffineTransformFromPrim(WLZ_TRANSFORM_2D_AFFINE,
    					      0.0, 0.0, 0.0,
					      1.0, 0.0, 0.0,
					      0.0, 0.0, 0.0, 0, &wlzErr);
  }
  if(newImageFile && (newTransform || transform) && (wlzErr == WLZ_ERR_NONE))
  {
    sec = (RecSection *)AlcMalloc(sizeof(RecSection));
  }
  if(sec == NULL)
  {
    if(newImageFile)
    {
      AlcFree(newImageFile);
    }
    if(newTransform)
    {
      WlzFreeAffineTransform(newTransform);
    }
  }
  else
  {
    sec->linkcount = 0;
    sec->index = index;
    sec->iterations = iterations;
    sec->correl = correlation;
    sec->obj = WlzAssignObject(obj, NULL);
    sec->imageFile = newImageFile;
    sec->transform = WlzAssignAffineTransform(transform? transform:
    							 newTransform, NULL);
    sec->transObj = NULL;
    sec->cumTransform = NULL;
    sec->cumTransObj = NULL;
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecMake FX 0x%lx\n",
	   (unsigned long )sec));
  return(sec);
}

/************************************************************************
* Function:	RecSecMakeEmpty					
* Returns:	RecSection *:		New registration section.
* Purpose:	Makes an empty registration section.		
* Global refs:	-						
* Parameters:	int index:		Section index.		
************************************************************************/
RecSection	*RecSecMakeEmpty(int index)
{
  RecSection	*sec = NULL;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecMakeEmpty FE %d\n",
	   index));
  sec = RecSecMake(index, 0, 0.0, "Empty", NULL, NULL);
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecMakeEmpty FX 0x%lx\n",
	   (unsigned long )sec));
  return(sec);
}


/************************************************************************
* Function:	RecSecFree					
* Returns:	void						
* Purpose:	Free's the given section.			
* Global refs:	-						
* Parameters:	RecSection *sec:	Section to AlcFree.	
************************************************************************/
void		RecSecFree(RecSection *sec)
{
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecFree FE 0x%lx\n",
	   (unsigned long )sec));
  if(sec)
  {
    REC_DBG((REC_DBG_SEC|REC_DBG_LVL_3),
	    ("RecSecFree 01 %d\n",
	     sec->linkcount));
    if(--(sec->linkcount) <= 0)
    {
      if(sec->imageFile)
      {
	AlcFree(sec->imageFile);
      }
      if(sec->transform)
      {
	REC_DBG((REC_DBG_SEC|REC_DBG_LVL_3),
		("RecSecFree 02 %d\n",
		 sec->transform->linkcount));
	(void )WlzFreeAffineTransform(sec->transform);
      }
      if(sec->obj)
      {
	REC_DBG((REC_DBG_SEC|REC_DBG_LVL_3),
		("RecSecFree 03 %d\n",
		 sec->obj->linkcount));
	(void )WlzFreeObj(sec->obj);
      }
      if(sec->transObj)
      {
	REC_DBG((REC_DBG_SEC|REC_DBG_LVL_3),
		("RecSecFree 04 %d\n",
		 sec->transObj->linkcount));
        (void )WlzFreeObj(sec->transObj);
      }
      if(sec->cumTransform)
      {
	REC_DBG((REC_DBG_SEC|REC_DBG_LVL_3),
		("RecSecFree 05 %d\n",
		 sec->cumTransform->linkcount));
	(void )WlzFreeAffineTransform(sec->cumTransform);
      }
      if(sec->cumTransObj && (sec->cumTransObj->linkcount > 0))
      {
	REC_DBG((REC_DBG_SEC|REC_DBG_LVL_3),
		("RecSecFree 05 %d\n",
		 sec->cumTransObj->linkcount));
        (void )WlzFreeObj(sec->cumTransObj);
      }
      AlcFree(sec);
    }
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecFree FX\n"));
}

/************************************************************************
* Function:	RecSecIsEmpty					
* Returns:	int:			Non zero if section is empty.
* Purpose:	Checks if this is an empty section by comparing the
*		image file name with the case insensitive string
*		'empty'.					
* Global refs:	-						
* Parameters:	RecSection *sec:	Section to be checked.	
************************************************************************/
int		RecSecIsEmpty(RecSection *sec)
{
  int		isEmpty = 0;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecIsEmpty FE 0x%lx\n",
	   (unsigned long )sec));
  if(sec)
  {
    if((sec->imageFile == NULL) || (*(sec->imageFile) == '\0') ||
       (strcasecmp(sec->imageFile, "Empty") == 0))
    {
      isEmpty = 1;
    }
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecIsEmpty FX %d\n",
	   isEmpty));
  return(isEmpty);
}

/************************************************************************
* Function:	RecSecNext					
* Returns:	RecSection *:		Next registration section,
*					or NULL if at end of list.
* Purpose:	Given a section list and a current item pointer, this
*		function sets the given destination pointer to the next
*		item and returns that list item's section entry.
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	Section list.		
*		HGUDlpListItem *cItem:	Current list item, NULL to
*					get first list entry.	
*		HGUDlpListItem **nItemP: Destination pointer for the
*					next list item, will be set to
*					NULL at end of list.	
*		int skipEmpty:		Skip over empty entries if flag
*					is non-zero.		
************************************************************************/
RecSection	*RecSecNext(HGUDlpList *secList, HGUDlpListItem *cItem,
			    HGUDlpListItem **nItemP, int skipEmpty)
{
  RecSection	*sec = NULL;
  HGUDlpListItem *item;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecNext FE 0x%lx 0x%lx 0x%lx %d\n",
	   (unsigned long )secList, (unsigned long )cItem,
	   (unsigned long )nItemP, skipEmpty));
  if(secList)
  {
    item = cItem;
    do
    {
      item = (item)? HGUDlpListNext(secList, item): HGUDlpListHead(secList);
      sec = (item)? (RecSection *)HGUDlpListEntryGet(secList, item): NULL;
    } while(skipEmpty && sec && RecSecIsEmpty(sec));
    if(nItemP)
    {
      *nItemP = item;
    }
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecNext FX 0x%lx\n",
	   (unsigned long )sec));
  return(sec);
}

/************************************************************************
* Function:	RecSecPrev					
* Returns:	RecSection *:		Previous registration section,
*					or NULL if at head of list.
* Purpose:	Given a section list and a current item pointer, this
*		function sets the given destination pointer to the
*		previous item and returns that list item's section
*		entry.						
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	Section list.		
*		HGUDlpListItem *cItem:	Current list item, NULL to
*					get last list entry.	
*		HGUDlpListItem **nItemP: Destination pointer for the
*					previous list item, will be
*					set to NULL at head of list.
*		int skipEmpty:		Skip over empty entries if flag
*					is non-zero.		
************************************************************************/
RecSection	*RecSecPrev(HGUDlpList *secList, HGUDlpListItem *cItem,
			    HGUDlpListItem **pItemP, int skipEmpty)
{
  RecSection	*sec = NULL;
  HGUDlpListItem *item;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecPrev FE 0x%lx 0x%lx 0x%lx %d\n",
	   (unsigned long )secList, (unsigned long )cItem,
	   (unsigned long )pItemP, skipEmpty));
  if(secList)
  {
    item = cItem;
    do
    {
      item = (item)? HGUDlpListPrev(secList, item): HGUDlpListTail(secList);
      sec = (item)? (RecSection *)HGUDlpListEntryGet(secList, item): NULL;
    } while(skipEmpty && sec && RecSecIsEmpty(sec));
    if(pItemP)
    {
      *pItemP = item;
    }
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecPrev FX 0x%lx\n",
	   (unsigned long )sec));
  return(sec);
}

/************************************************************************
* Function:	RecSecToStr					
* Returns:	char *:			New section string.	
* Purpose:	Creates a string from the given registration serial
*		section and bit mask for the fields required.	
* Global refs:	-						
* Parameters:	RecSection *sec:	Given section.		
*		unsigned int reqFields:	Bit mask for fields required
*					in the strings.		
*		char **eMsg:		Ptr for error message strings.
************************************************************************/
char		*RecSecToStr(RecSection *sec, unsigned int reqFields,
			     char **eMsg)
{
  unsigned long tUL0,
  		filePathLen,
  		strIdx;
  char		*secStr = NULL;
  char		strBuf[1024]; 	   /* This must be big enough for any string */

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecToStr FE 0x%lx %d 0x%lx %d\n",
	   (unsigned long )sec, reqFields, (unsigned long )eMsg));
  if(sec)
  {
    strIdx = 0;
    strBuf[0] = '\0';
    if(reqFields & REC_SECMSK_INDEX)
    {
      (void )sprintf(strBuf + strIdx, "Index % 12d", sec->index);
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_ITERATIONS)
    {
      (void )sprintf(strBuf + strIdx, ", Iterations % 12d",
		     sec->iterations);
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_CORREL)
    {
      (void )sprintf(strBuf + strIdx, ", Correlation % 12.8g",
		     sec->correl);
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_IMAGEFILE)
    {
      filePathLen = strlen(sec->imageFile);
      tUL0 = filePathLen % 10; 			       /* Pad out file paths */
      if(tUL0 == 0)
      {
        filePathLen += 10;
      }
      else
      {
        filePathLen += 20 - tUL0;
      }
      filePathLen = strlen(sec->imageFile) + 10;
      filePathLen -= filePathLen % 10;
      (void )sprintf(strBuf + strIdx, ", Imagefile %*s",
		     filePathLen,
		     sec->imageFile);	 	 /* Dynamic string precision */
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_TRANSF_TX)
    {
      (void )sprintf(strBuf + strIdx, ", Tx % 12.8g",
		     sec->transform->tx);
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_TRANSF_TY)
    {
      (void )sprintf(strBuf + strIdx, ", Ty % 12.8g",
		     sec->transform->ty);
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_TRANSF_TZ)
    {
      (void )sprintf(strBuf + strIdx, ", Tz % 12.8g",
		     sec->transform->tz);
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_TRANSF_SCALE)
    {
      (void )sprintf(strBuf + strIdx, ", Tscale % 12.8g",
		     sec->transform->scale);
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_TRANSF_THETA)
    {
      (void )sprintf(strBuf + strIdx, ", Ttheta % 12.8g",
		     sec->transform->theta);
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_TRANSF_PHI)
    {
      (void )sprintf(strBuf + strIdx, ", Tphi % 12.8g",
		     sec->transform->phi);
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_TRANSF_ALPHA)
    {
      (void )sprintf(strBuf + strIdx, ", Talpha % 12.8g",
		     sec->transform->alpha);
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_TRANSF_PSI)
    {
      (void )sprintf(strBuf + strIdx, ", Tpsi % 12.8g",
		     sec->transform->psi);
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_TRANSF_XSI)
    {
      (void )sprintf(strBuf + strIdx, ", Txsi % 12.8g",
		     sec->transform->xsi);
      strIdx = strlen(strBuf);
    }
    if(reqFields & REC_SECMSK_TRANSF_INVERT)
    {
      (void )sprintf(strBuf + strIdx, ", Tinvert %d",
		     (sec->transform->invert)? 1: 0);
      strIdx = strlen(strBuf);
    }
    REC_DBG((REC_DBG_SEC|REC_DBG_LVL_2),
	    ("RecSecListToStrList 01 >%s<\n",
	     strBuf? strBuf: "(null)"));
    if(strIdx > 0)
    {
      secStr = AlcStrDup(strBuf);
    }
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecToStr FX 0x%lx\n",
	   (secStr)? (secStr): ("null")));
  return(secStr);
}

/************************************************************************
* Function:	RecSecListToStrList				
* Returns:	RecError:		Non-zero if function fails.
* Purpose:	Creates a list of strings from a registration serial
*		section list and a bit mask for the fields required.
*		This is NOT intended to be used for output to a file
*		(RecFileSecWrite() should be used). This function was
*		written to allow a user to be presented with a simple 
*		list of serial sections within a GUI application.
* Global refs:	-						
* Parameters:	char ***strList:	Destination ptr for list of
*					strings.		
*		HGUDlpList *secList:	Given section list.	
*		int numSec:		Number of sections in secList.
*		char **eMsg:		Ptr for error message strings.
*		unsigned int reqFields:	Bit mask for fields required
*					in the strings.		
************************************************************************/
RecError	RecSecListToStrList(char ***strList, HGUDlpList *secList,
				    int numSec, char **eMsg,
				    unsigned int reqFields)
{
  int		secIdx = 0;
  HGUDlpListItem *secItem;
  RecSection	*sec;
  RecError	errFlag = REC_ERR_NONE;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecListToStrList FE 0x%lx 0x%lx %d 0x%lx %d\n",
	   (unsigned long )strList, (unsigned long )secList, numSec,
	   (unsigned long )eMsg, reqFields));
  if((*strList = (char **)AlcMalloc(sizeof(char **) *
  				 numSec)) != NULL)   /* Allocate string list */
  {
    secItem = HGUDlpListHead(secList);
    while(secItem &&
    	  ((sec = HGUDlpListEntryGet(secList, secItem)) != NULL) &&
    	  (secIdx < numSec) && (errFlag == REC_ERR_NONE))
    {
      if((*(*strList + secIdx) = RecSecToStr(sec, reqFields, eMsg)) == NULL)
      {
        errFlag = REC_ERR_MALLOC;
      }
      else
      {
	secItem = HGUDlpListNext(secList, secItem);
	++secIdx;
      }
    }
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecListToStrList FX %d\n",
	   errFlag));
  return(errFlag);
}

/************************************************************************
* Function:	RecSecListSort					
* Returns:	void						
* Purpose:	Sorts the given HGUDlpList section list using the given
*		section field mask.				
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	Given section list.	
*		unsigned int sortMask:	Bit mask for the section field
*					to sort on.		
************************************************************************/
void		RecSecListSort(HGUDlpList *secList, unsigned int sortMask)
{
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecListSort FE 0x%lx %d\n",
	   (unsigned long )secList, sortMask));
  sortMask &= REC_SECMSK_ALL;
  if(secList && sortMask)
  {
    switch(sortMask)
    {
      case REC_SECMSK_INDEX:
        (void )HGUDlpListSort(secList,  RecSecSortFnIndex);
	break;
      case REC_SECMSK_ITERATIONS:
        (void )HGUDlpListSort(secList,  RecSecSortFnIterations);
	break;
      case REC_SECMSK_CORREL:
        (void )HGUDlpListSort(secList,  RecSecSortFnCorrel);
	break;
      case REC_SECMSK_IMAGEFILE:
        (void )HGUDlpListSort(secList,  RecSecSortFnImageFile);
	break;
      case REC_SECMSK_TRANSF_TX:
        (void )HGUDlpListSort(secList,  RecSecSortFnTransfTx);
	break;
      case REC_SECMSK_TRANSF_TY:
        (void )HGUDlpListSort(secList,  RecSecSortFnTransfTy);
	break;
      case REC_SECMSK_TRANSF_TZ:
        (void )HGUDlpListSort(secList,  RecSecSortFnTransfTz);
	break;
      case REC_SECMSK_TRANSF_SCALE:
        (void )HGUDlpListSort(secList,  RecSecSortFnTransfScale);
	break;
      case REC_SECMSK_TRANSF_THETA:
        (void )HGUDlpListSort(secList,  RecSecSortFnTransfTheta);
	break;
      case REC_SECMSK_TRANSF_PHI:
        (void )HGUDlpListSort(secList,  RecSecSortFnTransfPhi);
	break;
      case REC_SECMSK_TRANSF_ALPHA:
        (void )HGUDlpListSort(secList,  RecSecSortFnTransfAlpha);
	break;
      case REC_SECMSK_TRANSF_PSI:
        (void )HGUDlpListSort(secList,  RecSecSortFnTransfPsi);
	break;
      case REC_SECMSK_TRANSF_XSI:
        (void )HGUDlpListSort(secList,  RecSecSortFnTransfXsi);
	break;
      case REC_SECMSK_TRANSF_INVERT:
        (void )HGUDlpListSort(secList,  RecSecSortFnTransfInvert);
	break;
    }
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecListSort FX\n"));
}

/************************************************************************
* Function:	RecSecSortFnSignCmpDbl				
* Returns:	int:			+1, 0 or -1		
* Purpose:	Given a pair of doubles, returns an integer with the
*		sign of their difference. Ie sign(d0 - d1).	
*		Used by some of the sort functions.		
* Global refs:	-						
* Parameters:	double d0:		First of the doubles.	
*		double d1:		Second of the doubles.	
************************************************************************/
static int	RecSecSortFnSignCmpDbl(double d0, double d1)
{
  int		sgnCmp = 0;

  if(d0 < d1)
  {
    sgnCmp = -1;
  }
  else if(d0 > d1)
  {
    sgnCmp = 1;
  }
  return(sgnCmp);
}

/************************************************************************
* Function:	RecSecSortFnIndex				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		indicies for HGUDlpListSort().			
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnIndex(void *entry0, void *entry1)
{
  int		tI0 = 0;
  RecSection	*sec0,
		*sec1;

  if(entry0 && entry1)
  {
    sec0 = (RecSection *)entry0,
    sec1 = (RecSection *)entry1;
    tI0 = sec1->index - sec0->index;
  }
  return(tI0);
}

/************************************************************************
* Function:	RecSecSortFnIterations				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		iterations for HGUDlpListSort().		
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnIterations(void *entry0, void *entry1)
{
  int		tI0 = 0;
  RecSection	*sec0,
		*sec1;

  if(entry0 && entry1)
  {
    sec0 = (RecSection *)entry0,
    sec1 = (RecSection *)entry1;
    tI0 = sec1->iterations - sec0->iterations;
  }
  return(tI0);
}

/************************************************************************
* Function:	RecSecSortFnCorrel				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		cross-correlations for HGUDlpListSort().	
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnCorrel(void *entry0, void *entry1)
{
  int		tI0 = 0;
  RecSection	*sec0,
		*sec1;

  if(entry0 && entry1)
  {
    sec0 = (RecSection *)entry0,
    sec1 = (RecSection *)entry1;
    tI0 = RecSecSortFnSignCmpDbl(sec1->correl, sec0->correl);
  }
  return(tI0);
}

/************************************************************************
* Function:	RecSecSortFnImageFile				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		image file names for HGUDlpListSort().		
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnImageFile(void *entry0, void *entry1)
{
  int		tI0 = 0;
  RecSection	*sec0,
		*sec1;

  if(entry0 && entry1)
  {
    sec0 = (RecSection *)entry0,
    sec1 = (RecSection *)entry1;
    if(sec0->imageFile && sec1->imageFile)
    {
      tI0 = strcmp(sec1->imageFile, sec0->imageFile);
    }
  }
  return(tI0);
}

/************************************************************************
* Function:	RecSecSortFnTransfTx				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		transform x components for HGUDlpListSort().	
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnTransfTx(void *entry0, void *entry1)
{
  return(RecSecSortFnTransf(entry0, entry1, REC_SECMSK_TRANSF_TX));
}

/************************************************************************
* Function:	RecSecSortFnTransfTy				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		transform y components for HGUDlpListSort().	
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnTransfTy(void *entry0, void *entry1)
{
  return(RecSecSortFnTransf(entry0, entry1, REC_SECMSK_TRANSF_TY));
}

/************************************************************************
* Function:	RecSecSortFnTransfTz				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		transform z components for HGUDlpListSort().	
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnTransfTz(void *entry0, void *entry1)
{
  return(RecSecSortFnTransf(entry0, entry1, REC_SECMSK_TRANSF_TZ));
}

/************************************************************************
* Function:	RecSecSortFnTransfScale				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		transform scale components for HGUDlpListSort().
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnTransfScale(void *entry0, void *entry1)
{
  return(RecSecSortFnTransf(entry0, entry1, REC_SECMSK_TRANSF_SCALE));
}

/************************************************************************
* Function:	RecSecSortFnTransfTheta				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		transform theta components for HGUDlpListSort().
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnTransfTheta(void *entry0, void *entry1)
{
  return(RecSecSortFnTransf(entry0, entry1, REC_SECMSK_TRANSF_THETA));
}

/************************************************************************
* Function:	RecSecSortFnTransfPhi				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		transform phi components for HGUDlpListSort().
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnTransfPhi(void *entry0, void *entry1)
{
  return(RecSecSortFnTransf(entry0, entry1, REC_SECMSK_TRANSF_PHI));
}

/************************************************************************
* Function:	RecSecSortFnTransfAlpha				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		transform alpha components for HGUDlpListSort().
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnTransfAlpha(void *entry0, void *entry1)
{
  return(RecSecSortFnTransf(entry0, entry1, REC_SECMSK_TRANSF_ALPHA));
}

/************************************************************************
* Function:	RecSecSortFnTransfPsi				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		transform psi components for HGUDlpListSort().
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnTransfPsi(void *entry0, void *entry1)
{
  return(RecSecSortFnTransf(entry0, entry1, REC_SECMSK_TRANSF_PSI));
}

/************************************************************************
* Function:	RecSecSortFnTransfXsi				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		transform xsi components for HGUDlpListSort().
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnTransfXsi(void *entry0, void *entry1)
{
  return(RecSecSortFnTransf(entry0, entry1, REC_SECMSK_TRANSF_XSI));
}

/************************************************************************
* Function:	RecSecSortFnTransfInvert			
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, compares the sections	
*		transform invert components for HGUDlpListSort().
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
************************************************************************/
static int	RecSecSortFnTransfInvert(void *entry0, void *entry1)
{
  return(RecSecSortFnTransf(entry0, entry1, REC_SECMSK_TRANSF_INVERT));
}

/************************************************************************
* Function:	RecSecSortFnTransf				
* Returns:	int:			>0, 0 or <0		
* Purpose:	Given two HGUDlpList entries which are known to be ptrs
*		to registration sections, and a registration section 
*		field mask, compares the sections transform fields
*		for RecSecSortFnTransf*()'s.			
* Global refs:	-						
* Parameters:	void *entry0:		First list entry.	
*		void *entry1:		Second list entry.	
*		unsigned int mask:	Mask for required field.
************************************************************************/
static int	RecSecSortFnTransf(void *entry0, void *entry1,
				   unsigned int mask)
{
  int		tI0 = 0;
  RecSection	*sec0,
		*sec1;
  WlzAffineTransform	*transf0,
  		*transf1;

  if(entry0 && entry1)
  {
    sec0 = (RecSection *)entry0,
    sec1 = (RecSection *)entry1;
    transf0 = sec0->transform;
    transf1 = sec1->transform;
    if(transf0 && transf1)
    {
      switch(mask)
      {
        case REC_SECMSK_TRANSF_TX:
          tI0 = RecSecSortFnSignCmpDbl(transf1->tx, transf0->tx);
	  break;
	case REC_SECMSK_TRANSF_TY:
          tI0 = RecSecSortFnSignCmpDbl(transf1->ty, transf0->ty);
	  break;
	case REC_SECMSK_TRANSF_TZ:
          tI0 = RecSecSortFnSignCmpDbl(transf1->tz, transf0->tz);
	  break;
	case REC_SECMSK_TRANSF_SCALE:
	  tI0 = transf1->scale - transf0->scale;
	  break;
	case REC_SECMSK_TRANSF_THETA:
          tI0 = RecSecSortFnSignCmpDbl(transf1->theta, transf0->theta);
	  break;
	case REC_SECMSK_TRANSF_PHI:
          tI0 = RecSecSortFnSignCmpDbl(transf1->phi, transf0->phi);
	  break;
	case REC_SECMSK_TRANSF_ALPHA:
          tI0 = RecSecSortFnSignCmpDbl(transf1->alpha, transf0->alpha);
	  break;
	case REC_SECMSK_TRANSF_PSI:
          tI0 = RecSecSortFnSignCmpDbl(transf1->psi, transf0->psi);
	  break;
	case REC_SECMSK_TRANSF_XSI:
          tI0 = RecSecSortFnSignCmpDbl(transf1->xsi, transf0->xsi);
	  break;
	case REC_SECMSK_TRANSF_INVERT:
	  tI0 = transf1->invert - transf0->invert;
	  break;
      }
    }
  }
  return(tI0);
}

/************************************************************************
* Function:	RecSecListIndiciesSet				
* Returns:	void						
* Purpose:	Sets the indicies for the sections in the given section
*		list using the given first index value and the given 
*		index increment.				
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	Given section list.	
*		int indexFirst:		First index value.	
*		int indexInc:		Index increment between the
*					sections.		
************************************************************************/
void		RecSecListIndiciesSet(HGUDlpList *secList,
				      int indexFirst, int indexInc)
{
  HGUDlpListItem *head;
  RecSecListIndexArgs args;

  args.indexCur = indexFirst;
  args.indexInc = indexInc;
  if(secList)
  {
    head = HGUDlpListHead(secList);
    (void )HGUDlpListIterate(secList, head, HGU_DLPLIST_DIR_TOTAIL,
			     RecSecListIndexFn, (void *)&args);
  }
}

/************************************************************************
* Function:	RecSecListIndexFn				
* Returns:	void						
* Purpose:	Iterated function for section lists associated with
*		RecSecListIndiciesSet() which sets the section index
*		of the given item.				
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	Given section list.	
*		HGUDlpListItem *item:	List item with section entry.
*		void *data:		Used to pass other args.
************************************************************************/
static int	RecSecListIndexFn(HGUDlpList *secList, HGUDlpListItem *item,
				  void *data)
{
  int		continueFlag = 0;
  RecSecListIndexArgs *args;
  RecSection	*sec;

  if(secList && item && data)
  {
    args = (RecSecListIndexArgs *)data;
    sec = (RecSection *)HGUDlpListEntryGet(secList, item);
    sec->index = args->indexCur;
    args->indexCur += args->indexInc;
    continueFlag = 1;
  }
  return(continueFlag);
}

/************************************************************************
* Function:	RecSecListInvalid				
* Returns:	unsigned int:		Registration section mask with
*					any fields that are INVALID
*					set.			
* Purpose:	Checks the registration section list for validity.
*		Only section fields which are indicated in the given 
*		section mask are checked for validity.		
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	Given section list.	
*		RecSection *invalidSec:	Destination pointer for the 
*					first invalid section list item
*					found, maybe NULL if not 
*					required.		
*		unsigned int mask:	Mask specifying which fields
*					are to be checked for validity.
************************************************************************/
unsigned int	 RecSecListInvalid(HGUDlpList *secList,
				   HGUDlpListItem **invalidItem,
				   unsigned int mask)
{
  HGUDlpListItem *item;
  RecSecListInvalidArgs args;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecListInvalid FE 0x%lx 0x%lx %d\n",
	   (unsigned long )secList, (unsigned long )invalidItem,
	   (unsigned long )mask));
  args.checkMask = mask;
  args.invalidMask = REC_SECMSK_NONE;
  args.prevSec = NULL;
  if(secList)
  {
    item = HGUDlpListHead(secList);
    item = HGUDlpListIterate(secList, item, HGU_DLPLIST_DIR_TOTAIL,
			     RecSecListInvalidFn, (void *)&args);
    if(args.prevSec)
    {
      RecSecFree(args.prevSec);
    }
    if(invalidItem != NULL)
    {
      if(args.invalidMask == REC_SECMSK_NONE)
      {
        *invalidItem = NULL;
      }
      else
      {
        *invalidItem = item;
      }
    }
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecListInvalid FX %d\n",
	   (unsigned long )(args.invalidMask)));
  return(args.invalidMask);
}

/************************************************************************
* Function:	RecSecListInvalidFn				
* Returns:	void						
* Purpose:	Iterated function for section lists associated with
*		RecSecListInvalid() which checks the validity of the
*		given section item.				
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	Given section list.	
*		HGUDlpListItem *item:	List item with section entry.
*		void *data:		Used to pass other args.
************************************************************************/
static int	RecSecListInvalidFn(HGUDlpList *secList, HGUDlpListItem *item,
				    void *data)
{
  int		continueFlag = 0;
  RecSecListInvalidArgs *args;
  RecSection	*sec;

  if(secList && item && data)
  {
    args = (RecSecListInvalidArgs *)data;
    sec = (RecSection *)HGUDlpListEntryGet(secList, item);
    if(args->checkMask & REC_SECMSK_INDEX)
    {
      if((sec->index < 0) ||
         ((args->prevSec) && (sec->index <= args->prevSec->index)))
      {
	args->invalidMask |= REC_SECMSK_INDEX;
      }
    }
    if(args->checkMask & REC_SECMSK_ITERATIONS)
    {
      if(sec->iterations < 0)
      {
        args->invalidMask |= REC_SECMSK_ITERATIONS;
      }
    }
    if(args->checkMask & REC_SECMSK_CORREL)
    {
      if(sec->correl < 0.0)
      {
        args->invalidMask |=  REC_SECMSK_CORREL;
      }
    }
    if(args->checkMask & REC_SECMSK_IMAGEFILE)
    {
      if((sec->imageFile == NULL) || (strlen(sec->imageFile) == 0))
      {
        args->invalidMask |= REC_SECMSK_IMAGEFILE;
      }
    }
    if(args->prevSec)
    {
      RecSecFree(args->prevSec);
    }
    args->prevSec = RecSecDup(sec);
    if(args->invalidMask == REC_SECMSK_NONE)
    {
      continueFlag = 1;
    }
  }
  return(continueFlag);
}

/************************************************************************
* Function:	RecSecAppendListFromFiles			
* Returns:	RecError:		Non-zero if function fails.
* Purpose:	Appends new section records to the given section list
*		using the given list of image file names. The section
*		indicies are set using the given first index and index
*		increment values. The new section records have identity
*		transforms, zero iteration count and cross correlation
*		values. The woolz image object is set to NULL.	
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	Section list to which the new
*					sections should be appended.
*		HGUDlpListItem *secmark: List item after which the new
*					sections should be appended, if
*					this is NULL the the new
*					sections are appended at the
*					end of the list.	
*		char **fileVec:		Vector of file name strings.
*		int nFiles:		Number of file names.	
*		int indexFirst:		Index value of first section of
*					the list.		
*		int indexInc:		Index increment value.	
************************************************************************/
RecError	 RecSecAppendListFromFiles(HGUDlpList *secList,
					   HGUDlpListItem *secMark,
					   char **fileVec, int nFiles,
					   int indexFirst, int indexInc)
{
  int		index;
  RecSection	*sec;
  RecError	errFlag = REC_ERR_NONE;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecCreateListFromFiles FE 0x%lx 0x%lx 0x%lx %d %d %d\n",
	   (unsigned long )secList, (unsigned long )fileVec,
	   (unsigned long )secMark, nFiles, indexFirst, indexInc));
  if((secList == NULL) || (fileVec == NULL) || (nFiles <= 0) ||
     (nFiles > REC_MAX_LIST))
  {
    errFlag = REC_ERR_FUNC;
  }
  else
  {
    index = indexFirst;
    while((errFlag == REC_ERR_NONE) && *fileVec && (nFiles > 0))
    {
      if(((sec = RecSecMake(index, 0, 0.0, *fileVec, NULL, NULL)) != NULL) &&
         ((secMark = HGUDlpListAppend(secList, secMark, sec,
	 			      (void (*)(void *) )RecSecFree)) != NULL))
      {
	--nFiles;
	++fileVec;
        index += indexInc;
      }
      else
      {
        errFlag = REC_ERR_MALLOC;
      }
    }
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecCreateListFromFiles FX %d\n",
	   errFlag));
  return(errFlag);
}

/************************************************************************
* Function:	RecSecCumTransfClear				
* Returns:	RecError:		Non-zero if function fails.
* Purpose:	Sets the cumulative transform field of each section in
*		the section list to NULL, from the given section to the
*		tail of the list. When set to NULL the cumulative
*		transforms then need recalculating prior to section
*		display, ie NULL is used to mark the cumulative 
*		transforms as invalid/outdated.			
*		If the given section item is NULL then all list items
*		are processed.					
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	The section list.	
*		HGUDlpListItem *secItem: Given section item.	
************************************************************************/
RecError	 RecSecCumTransfClear(HGUDlpList *secList,
			    	      HGUDlpListItem *secItem)
{
  RecError	errFlag = REC_ERR_NONE;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecCumTransfClear FE 0x%lx 0x%lx\n",
	   (unsigned long )secList, (unsigned long )secItem));
  if(secList)
  {
    (void )HGUDlpListIterate(secList, secItem, HGU_DLPLIST_DIR_TOTAIL,
    		      	     RecSecCumTransfClearItFn, NULL);
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecCumTransfClear FX %d\n",
	   errFlag));
  return(errFlag);
}

/************************************************************************
* Function:	RecSecCumTransfClearItFn			
* Returns:	int:			Non-zero to continue iteration.
* Purpose:	Iterated function which clears the cumulative transform
*		and cumulative transform transformed object fields of
*		the given section list item to be NULL.		
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	The section list.	
*		HGUDlpListItem *secItem: Given section item.	
*		void *itData:		Not used.		
************************************************************************/
static int	RecSecCumTransfClearItFn(HGUDlpList *secList,
					 HGUDlpListItem *secItem,
					void *itData)
{
  int		iterate = 0;
  RecSection	*sec;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_3),
          ("RecSecCumTransfClearItFn FE 0x%lx 0x%lx 0x%lx\n",
	   (unsigned long )secList, (unsigned long )secItem,
	   (unsigned long )itData));
  if(secList && secItem &&
     ((sec = (RecSection *)HGUDlpListEntryGet(secList, secItem)) != NULL))
  {
    if(sec->cumTransObj)
    {
      (void )WlzFreeObj(sec->cumTransObj);
      sec->cumTransObj = NULL;
    }
    if(sec->cumTransform)
    {
      (void )WlzFreeAffineTransform(sec->cumTransform);
      sec->cumTransform = NULL;
    }
    iterate = 1;
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_3),
          ("RecSecCumTransfClearItFn FX %d\n",
	   iterate));
  return(iterate);
}

/************************************************************************
* Function:	RecSecCumTransfSet				
* Returns:	RecError:		Non-zero if function fails.
* Purpose:	Calculates and sets the cumulative transform field of
*		each section in the section list from the head of
*		the list to the given section. If a section has a 
*		non-NULL cumulative transform then it is assumed valid.
*		This function should be called AFTER the function
*		RecSecCumTransfClear has been called to set invalid
*		entries to NULL.				
*		If the given section item is NULL then all entries in
*		the list will be processed.			
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	The section list.	
*		HGUDlpListItem *secItem: Given section item.	
************************************************************************/
RecError	 RecSecCumTransfSet(HGUDlpList *secList,
		 		   HGUDlpListItem *secItem)
{
  RecError	errFlag = REC_ERR_NONE;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecCumTransfSet FE 0x%lx 0x%lx\n",
	   (unsigned long )secList, (unsigned long )secItem));
  if(secList)
  {
    (void )HGUDlpListIterate(secList, NULL, HGU_DLPLIST_DIR_TOTAIL,
    		      	     RecSecCumTransfSetItFn, secItem);
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
          ("RecSecCumTransfSet FX %d\n",
	   errFlag));
  return(errFlag);
}

/************************************************************************
* Function:	RecSecCumTransfSetItFn				
* Returns:	int:			Non-zero to continue iteration.
* Purpose:	Iterated function which sets the cumulative transform 
*		fields of a section list, working from the head of the
*		list towards the tail and stoping when the specified 
*		list item has been processed.			
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	The section list.	
*		HGUDlpListItem *secItem: Given section item.	
*		void *itData:		Used to pass item to stop at.
************************************************************************/
static int	RecSecCumTransfSetItFn(HGUDlpList *secList,
					 HGUDlpListItem *secItem,
					void *itData)
{
  int		iterate = 0;
  HGUDlpListItem *prevItem,
  		*stopItem;
  WlzAffineTransform	*cumTransf = NULL;
  RecSection	*curSec,
  		*prevSec;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_2),
          ("RecSecCumTransfSetItFn FE 0x%lx 0x%lx 0x%lx\n",
	   (unsigned long )secList, (unsigned long )secItem,
	   (unsigned long )itData));
  if(secList && secItem && itData)
  {
    stopItem = (HGUDlpListItem *)itData;
    curSec = (RecSection *)HGUDlpListEntryGet(secList, secItem);
    if(curSec && curSec->transform)		/* Should always be true */
    {
      if(HGUDlpListItemIsHead(secList, secItem))
      {
	if(curSec->cumTransform == NULL)
	{
	  curSec->cumTransform = curSec->transform;
	  ++(curSec->cumTransform->linkcount);
	}
	cumTransf = curSec->cumTransform;
      }
      else
      {
	if(curSec->cumTransform == NULL)
	{
	  prevItem = HGUDlpListPrev(secList, secItem);
	  prevSec = (RecSection *)HGUDlpListEntryGet(secList, prevItem);
	  if(prevSec && prevSec->cumTransform)      /* Should always be true */
	  {
	    /* HACK NEW CODE */
	    cumTransf = WlzAffineTransformProduct(curSec->transform,
	    					  prevSec->cumTransform,
						  NULL);
	    /* HACK NEW CODE */
	    /* HACK OLD CODE
	    cumTransf = WlzAffineTransformProduct(prevSec->cumTransform,
					          curSec->transform,
						  NULL);
	    HACK OLD CODE */
	    curSec->cumTransform = cumTransf;
	    ++(curSec->cumTransform->linkcount);
	  }
	}
	else
	{
	  cumTransf = curSec->cumTransform;
	}
      }
      REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_2),
	      ("RecSecCumTransfSetItFn 02 0x%lx\n",
	       (unsigned long )cumTransf));
      if(cumTransf)
      {
	REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_3),
		("RecSecCumTransfSetItFn 02 %d %g %g %g %g %g %g %g %g %g %d\n",
		 cumTransf->type,
		 cumTransf->tx,
		 cumTransf->ty,
		 cumTransf->tz,
		 cumTransf->scale,
		 cumTransf->theta,
		 cumTransf->phi,
		 cumTransf->alpha,
		 cumTransf->psi,
		 cumTransf->xsi,
		 cumTransf->invert));
	if(secItem != stopItem)
	{
	  iterate = 1;
	}
      }
    }
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_2),
          ("RecSecCumTransfSetItFn FX %d\n",
	   iterate));
  return(iterate);
}

/************************************************************************
* Function:	RecSecFindItemIndex				
* Returns:	HGUDlpListItem *:	First item found with matching 
*					index or NULL if no matching
*					index (or on error).	
* Purpose:	Finds the first item in the given section list with a
*		matching index and returns a pointer to the list item.
* Global refs:	-						
* Parameters:	HGUDlpList *list:	Section list to search.	
*		HGUDlpListItem *item:	Item to search first and from.
*		int index:		Section index to search for.
*		HGUDlpListDirection dir: Search direction.	
************************************************************************/
HGUDlpListItem	*RecSecFindItemIndex(HGUDlpList *list, HGUDlpListItem *from,
				     int index, HGUDlpListDirection dir)
{
  HGUDlpListItem *found = NULL;
  RecSection 	*sec;

  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecFindItemIndex FE 0x%lx 0x%lx %d %d\n",
	   (unsigned long )list, (unsigned long )from, index, (int )dir));
  if(list && 
     ((dir == HGU_DLPLIST_DIR_TOTAIL) || (dir == HGU_DLPLIST_DIR_TOHEAD)))
  {
    found = HGUDlpListIterate(list, from, dir, RecSecFindItemIndexItFn,
    			      (void *)&index);
    if(found)			    /* Check for indicies same and not error */
    {
      sec = (RecSection *)HGUDlpListEntryGet(list, found);
      if((sec == NULL) || (sec->index != index))
      {
        found = NULL;
      }
    }
  }
  REC_DBG((REC_DBG_SEC|REC_DBG_LVL_FN|REC_DBG_LVL_1),
	  ("RecSecFindItemIndex FX 0x%lx\n",
	   (unsigned long )found));
  return(found);
}

/************************************************************************
* Function:	RecSecFindItemIndexItFn				
* Returns:	int:			Non zero if the item's index is
*					equal to the given index.
* Purpose:	Iterated function for RecSecFindItemIndex().	
* Global refs:	-						
* Parameters:	HGUDlpList *secList:	The section list.	
*		HGUDlpListItem *secItem: The given item.	
*		void *data:		Used to pass given index.
************************************************************************/
static int	RecSecFindItemIndexItFn(HGUDlpList *secList,
					HGUDlpListItem *secItem,
				  	void *data)
{
  int		notEqual = 0;
  RecSection	*sec;

  if(secList && secItem)
  {
    sec = (RecSection *)HGUDlpListEntryGet(secList, secItem);
    if(sec)
    {
      notEqual = sec->index != *(int *)data;
    }
  }
  return(notEqual);
}
