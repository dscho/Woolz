#pragma ident "MRC HGU $Id$"
/*!
* \file         WlzCCor.c
* \author       Bill Hill
* \date         August 2003
* \version      $Id$
* \note
*               Copyright
*               2003 Medical Research Council, UK.
*               All rights reserved.
*               All rights reserved.
* \par Address:
*               MRC Human Genetics Unit,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \brief	Computes the cross correlation of two objects.
* \ingroup	WlzFeatures
* \todo         -
* \bug          None known.
*/
#include <float.h>
#include <limits.h>
#include <Wlz.h>

/*!
* \return	Cross correlation value.
* \ingroup	WlzFeatures
* \brief	Computes the cross correlation of the two given 2D
*		spatial domain objects in the spatial domain.
*		The algorithm used by this function is not an efficient
*		unless a single cross correlation value is required.
* \param	obj0			First object. Must have been assigned.
* \param	obj1			Second object. Must have been assigned.
* \param	unionFlg		Computes the cross correlation value
*					in the union of the two objects
*					domains if non zero. The default is
*					to use the intersection of the
*					domains.
* \param	dstErr			Destination error pointer,
*                                       may be NULL.
*/
double		WlzCCorS2D(WlzObject *obj0, WlzObject *obj1,
			   int unionFlg, WlzErrorNum *dstErr)
{
  double	gV0,
  		gV1,
		gV2,
		cCor = 0.0;
  WlzIVertex2	pos;
  WlzGreyValueWSpace *gVWSp0 = NULL,
  		*gVWSp1 = NULL;
  WlzIntervalWSpace iWSp2;
  WlzObject	*obj2 = NULL;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((obj0 == NULL) || (obj1 == NULL))
  {
    errNum = WLZ_ERR_OBJECT_NULL;
  }
  else if((obj0->type != WLZ_2D_DOMAINOBJ) || (obj1->type != WLZ_2D_DOMAINOBJ))
  {
    errNum = WLZ_ERR_OBJECT_TYPE;
  }
  else if((obj0->domain.core == NULL) || (obj1->domain.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else if((obj0->values.core == NULL) || (obj1->values.core == NULL))
  {
    errNum = WLZ_ERR_VALUES_NULL;
  }
  else
  {
    obj2 = (unionFlg)?
    	   WlzUnion2(obj0, obj1, &errNum):
	   WlzIntersect2(obj0, obj1, &errNum);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    errNum = WlzInitRasterScan(obj2, &iWSp2, WLZ_RASTERDIR_ILIC);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    gVWSp0 = WlzGreyValueMakeWSp(obj0, &errNum);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    gVWSp1 = WlzGreyValueMakeWSp(obj1, &errNum);
  }
  while((errNum == WLZ_ERR_NONE) &&
	((errNum = WlzNextInterval(&iWSp2)) == WLZ_ERR_NONE))
  {
    gV2 = 0.0;
    pos.vtY = iWSp2.linpos;
    for(pos.vtX = iWSp2.lftpos; pos.vtX <= iWSp2.rgtpos; ++(pos.vtX))
    {
      WlzGreyValueGet(gVWSp0, 0, pos.vtY, pos.vtX);
      switch(gVWSp0->gType)
      {
        case WLZ_GREY_INT:
	  gV0 = (*(gVWSp0->gVal)).inv;
	  break;
	case WLZ_GREY_SHORT:
	  gV0 = (*(gVWSp0->gVal)).shv;
	  break;
	case WLZ_GREY_UBYTE:
	  gV0 = (*(gVWSp0->gVal)).ubv;
	  break;
	case WLZ_GREY_FLOAT:
	  gV0 = (*(gVWSp0->gVal)).flv;
	  break;
	case WLZ_GREY_DOUBLE:
	  gV0 = (*(gVWSp0->gVal)).dbv;
	  break;
	default:
	  errNum = WLZ_ERR_GREY_TYPE;
	  break;
      }
      WlzGreyValueGet(gVWSp1, 0, pos.vtY, pos.vtX);
      switch(gVWSp1->gType)
      {
        case WLZ_GREY_INT:
	  gV1 = (*(gVWSp1->gVal)).inv;
	  break;
	case WLZ_GREY_SHORT:
	  gV1 = (*(gVWSp1->gVal)).shv;
	  break;
	case WLZ_GREY_UBYTE:
	  gV1 = (*(gVWSp1->gVal)).ubv;
	  break;
	case WLZ_GREY_FLOAT:
	  gV1 = (*(gVWSp1->gVal)).flv;
	  break;
	case WLZ_GREY_DOUBLE:
	  gV1 = (*(gVWSp1->gVal)).dbv;
	  break;
	default:
	  errNum = WLZ_ERR_GREY_TYPE;
	  break;
      }
      gV2 += gV0 * gV1;
    }
    cCor += gV2;
  }
  if(errNum == WLZ_ERR_EOO)
  {
    errNum = WLZ_ERR_NONE;
  }
  WlzFreeObj(obj2);
  WlzGreyValueFreeWSp(gVWSp0);
  WlzGreyValueFreeWSp(gVWSp1);
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(cCor);
}