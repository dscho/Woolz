#if defined(__GNUC__)
#ident "University of Edinburgh $Id$"
#else
static char _WlzIntervalCount_c[] = "University of Edinburgh $Id$";
#endif
/*!
* \file         libWlz/WlzIntervalCount.c
* \author       Bill Hill
* \date         September 2003
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
* \brief	Counts the number of intervals (or equivalent)
* 		in an object's domain.
* \ingroup	WlzDomainOps
*/

#include <Wlz.h>

/*!
* \return	Maximum number of intervals on a line.
* \ingroup	WlzDomainOps
* \brief	Computes the maximum number of intervals on any line.
* \param	iDom			Given interval domain.
*/
int		WlzIDomMaxItvLn(WlzIntervalDomain *iDom)
{
  int		ln,
		nLn,
		nItv,
  		maxItv = 0;
  WlzIntervalLine *iLn;

  if(iDom != NULL)
  {
    switch(iDom->type)
    {
      case WLZ_INTERVALDOMAIN_INTVL:
	nLn = iDom->lastln - iDom->line1 + 1;
	iLn = iDom->intvlines;
	for(ln = 0; ln < nLn; ++ln)
	{
	  nItv = (iLn + ln)->nintvs;
	  if(nItv > maxItv)
	  {
	    maxItv = nItv;
	  }
	}
        break;
      case WLZ_INTERVALDOMAIN_RECT:
	maxItv = 1;
        break;
      default:
        break;
    }
  }
  return(maxItv);
}

/*! 
* \ingroup      WlzDomainOps
* \brief        Count the number of intervals or equivalent if rectangular.
*
* \return       Number of intervals if the domain type is
 <tt>WLZ_INTERVALDOMAIN_INTVL</tt> else the number of lines.
* \param    idom	Input domain.
* \param    wlzErr	Error return.
* \par      Source:
*                WlzIntervalCount.c
*/
int WlzIntervalCount(WlzIntervalDomain *idom, WlzErrorNum *wlzErr)
{
  int		l,
		ll,
		intcount = -1;
  WlzIntervalLine *intl;
  WlzErrorNum	errNum = WLZ_ERR_UNSPECIFIED;

  if(idom == NULL)
  {
    errNum = WLZ_ERR_INTERVALDOMAIN_NULL;
  }
  else
  {
    switch(idom->type)
    {
      case WLZ_INTERVALDOMAIN_INTVL:
	intl = idom->intvlines;
	intcount = 0;
	ll = idom->lastln;
	for (l = idom->line1; l <= ll; ++l)
	{
	  intcount += (intl++)->nintvs;
	}
	errNum = WLZ_ERR_NONE;
	break;
      case WLZ_INTERVALDOMAIN_RECT:
	errNum = WLZ_ERR_NONE;
	intcount = idom->lastln - idom->line1 + 1;
	break;
      default:
	errNum = WLZ_ERR_INTERVALDOMAIN_TYPE;
	break;
    }
  }
  if(wlzErr)
  {
    *wlzErr = errNum;
  }
  return(intcount);
}
