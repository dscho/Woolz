/************************************************************************
* Project:      Java Woolz
* Title:        WlzObject.java
* Date:         January 1999
* Purpose:      Java binding for Woolz object.
* Copyright:	1997 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Maintenance:	Log changes below, with most recent at top of list.
* @author       Bill Hill (bill@hgu.mrc.ac.uk)
* @version 	MRC HGU %I%, %G%
************************************************************************/
package uk.ac.mrc.hgu.Wlz;
import uk.ac.mrc.hgu.Wlz.*;

public class WlzObject extends WlzPointer
{

#include "C2Java/Binding/WlzObjectC2J.javapp"

  /**********************************************************************
  * Purpose:    Free's the Woolz object using WlzFreeObj().
  * @return     void
  * @param:     void
  **********************************************************************/
  protected void finalize() throws WlzException
  {
    if(this.isNull() != true)
    {
      WlzObject.WlzFreeObj(this);
      this.clear();
    }
  }

}

