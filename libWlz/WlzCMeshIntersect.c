#if defined(__GNUC__)
#ident "University of Edinburgh $Id$"
#else
static char _WlzCMeshIntersect_c[] = "University of Edinburgh $Id$";
#endif
/*!
* \file         libWlz/WlzCMeshIntersect.c
* \author       Bill Hill
* \date         February 2012
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
* \brief	Functions for computing the intersection with conforming
* 		meshes.
* \ingroup 	WlzMesh
*/
#include <Wlz.h>
#include <float.h>

#ifdef WLZ_UNUSED_FUNCTIONS
static int			WlzIntersectDomAABBTri3D(
				  WlzObject *dObj,
				  WlzDVertex3 p0,
				  WlzDVertex3 p1,
				  WlzDVertex3 p2,
				  WlzErrorNum *dstErr);
#endif /* WLZ_UNUSED_FUNCTIONS */
static int			WlzCMeshIntersectSortVtx2D(
				  void *data,
				  int *idx,
				  int id0,
				  int id1);
static int			WlzCMeshIntersectDomIsInside3D(
				  WlzObject *sObj,
				  WlzDVertex3 q,
				  WlzDVertex3 nrm,
				  double delta);

/*!
* \return	New 2D spatial domain obect.
* \ingroup	WlzMesh
* \brief	Computes the intersection of a 3D spatial domain object
* 		with a 2.5D conforming mesh that has displacements to a
* 		plane. The result is a 2D domain coresponding to the
* 		intersection mapped to the plane.
* \param	sObj			Object with 3D spatial domain.
* \param	cObj			Object with the 2D5 mesh and
* 					displacements to a plane.
* \param	delta			Distance normal to the surface
* 					within which intersection is allowed.
* \param	scale			Additional scale factor from mesh
* 					to spatial domain.
* \param	dstErr			Destination error pointer, may be NULL.
*/
WlzObject	*WlzCMeshIntersectDom2D5(WlzObject *sObj, WlzObject *cObj,
					double delta, double scale,
					WlzErrorNum *dstErr)
{
  int		idN;
  WlzCMesh2D5	*mesh;
  WlzIndexedValues *ixv;
  WlzObject	*fObj = NULL,
  		*rObj = NULL;
  WlzObject	*tObj[5];	         	       /* Temporary objects. */
  WlzGreyValueWSpace *gVWSp = NULL;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  const double 	eps = WLZ_MESH_TOLERANCE;

  delta = fabs(delta);
  for(idN = 0; idN < 5; ++idN)
  {
    tObj[idN] = NULL;
  }
  /* Check given parameters. */
  if((sObj == NULL) || (cObj == NULL))
  {
    errNum = WLZ_ERR_OBJECT_NULL;
  }
  else if ((sObj->type != WLZ_3D_DOMAINOBJ) || (cObj->type != WLZ_CMESH_2D5))
  {
    errNum = WLZ_ERR_OBJECT_TYPE;
  }
  else if ((sObj->domain.core == NULL) ||
           ((mesh = cObj->domain.cm2d5) == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else if((sObj->domain.core->type != WLZ_PLANEDOMAIN_DOMAIN) ||
          (mesh->type != WLZ_CMESH_2D5))
  {
    errNum = WLZ_ERR_DOMAIN_TYPE;
  }
  else if((ixv = cObj->values.x) == NULL)
  {
    errNum = WLZ_ERR_VALUES_NULL;
  }
  else if(ixv->type != WLZ_INDEXED_VALUES)
  {
    errNum = WLZ_ERR_VALUES_TYPE;
  }
  else if((ixv->rank != 1) || (ixv->dim[0] < 2))
  {
    errNum = WLZ_ERR_VALUES_DATA;
  }
  else if(fabs(scale) < eps)
  {
    errNum = WLZ_ERR_PARAM_DATA;
  }
  /* Create a scaled object corresponding to the flattened mesh with all values
   * ubyte and set to zero. */
  if(errNum == WLZ_ERR_NONE)
  {
    if(errNum == WLZ_ERR_NONE)
    {
      tObj[0] = WlzCMeshExtract2D(cObj, 1, &errNum);
    }
    if(errNum == WLZ_ERR_NONE)
    {
      tObj[1] = WlzCMeshToDomObj(tObj[0], 0, scale, &errNum);
    }
    if(errNum == WLZ_ERR_NONE)
    {
      WlzPixelV	v0;
      WlzValues val;
      WlzObjectType gTType;

      val.core = NULL;
      v0.type = WLZ_GREY_UBYTE;
      v0.v.ubv = 0;
      gTType = WlzGreyTableType(WLZ_GREY_TAB_RAGR, WLZ_GREY_UBYTE, NULL);
      val.v = WlzNewValueTb(tObj[1], gTType, v0, &errNum);
      if(errNum == WLZ_ERR_NONE)
      {
        fObj = WlzAssignObject(
	       WlzMakeMain(WLZ_2D_DOMAINOBJ, tObj[1]->domain, val, NULL, NULL,
			   &errNum), NULL);
      }
      if(errNum != WLZ_ERR_NONE)
      {
        (void )WlzFreeValues(val);
      }
    }
  }
  for(idN = 0; idN < 2; ++idN)
  {
    (void )WlzFreeObj(tObj[idN]);
    tObj[idN] = NULL;
  }
  if(errNum == WLZ_ERR_NONE)
  {
    gVWSp = WlzGreyValueMakeWSp(fObj, &errNum);
  }
  /* For each element of the given mesh, ask does it intersect the given
   * domain. If it does set the appropriate pixel values in the flat
   * object. */
  if(errNum == WLZ_ERR_NONE)
  {
    int		idE;

    for(idE = 0; idE < mesh->res.elm.maxEnt; ++idE)
    {
      WlzCMeshElm2D5 *elm;

      elm = (WlzCMeshElm2D5 *)AlcVectorItemGet(mesh->res.elm.vec, idE);
      if(elm->idx >= 0)
      {
	WlzDVertex3 nrm;
	WlzDVertex3 dLim[2],
		    pos3[3];
	WlzCMeshNod2D5 *nod[3];


	WlzCMeshElmGetNodes2D5(elm, nod + 0, nod + 1, nod + 2);
	pos3[0]= nod[0]->pos; pos3[1]= nod[1]->pos; pos3[2]= nod[2]->pos;
	dLim[0].vtX = sObj->domain.p->kol1;
	dLim[0].vtY = sObj->domain.p->line1;
	dLim[0].vtZ = sObj->domain.p->plane1;
	dLim[1].vtX = sObj->domain.p->lastkl;
	dLim[1].vtY = sObj->domain.p->lastln;
	dLim[1].vtZ = sObj->domain.p->lastpl;
	if(delta > WLZ_MESH_TOLERANCE)
	{
          WlzDVertex3 dNrm;

	  nrm = WlzGeomTriangleNormal(pos3[0], pos3[1], pos3[2]);
	  WLZ_VTX_3_SCALE(dNrm, nrm, delta);
	  WLZ_VTX_3_SUB(dLim[0], dLim[0], dNrm);
	  WLZ_VTX_3_ADD(dLim[1], dLim[1], dNrm);
	}
	/* If the element does not intersect the bounding box of the
	 * spatial domain (expanded by the normal's projection scaled by
	 * delta) then there can be no intersection. */
        if(WlzGeomTriangleAABBIntersect3D(pos3[0], pos3[1], pos3[2],
	                                  dLim[0], dLim[1], 0) != 0)
	{
	  double	d,
		  	x2,
			y2;
	  int 		idx[3];
	  WlzDVertex2 	p2[4], /* Positions of the 2D vertices, sorted by
				  line then column, first is absolute,
				  rest are relative to the first. */
			  pos2[3]; /* Positions of the 2D vertices. */
	  WlzDVertex3	p3[3];

	  /* Compute the coordinates of the element's nodes in the plane. */
	  for(idN = 0; idN < 3; ++idN)
	  {
	    double 	*dsp;

	    idx[idN] = idN;
	    dsp = (double *)WlzIndexedValueGet(ixv, nod[idN]->idx);
	    pos2[idN].vtX = (pos3[idN].vtX + dsp[0]) * scale;
	    pos2[idN].vtY = (pos3[idN].vtY + dsp[1]) * scale;
	  }
	  /* Sort the vertices by increasing line then column. */
	  (void )AlgHeapSortIdx(pos2, idx, 3, WlzCMeshIntersectSortVtx2D);
	  p2[0] = pos2[idx[0]];
	  p2[1] = pos2[idx[1]];
	  p2[3] = p2[2] = pos2[idx[2]];
	  WLZ_VTX_2_SUB(p2[1], p2[1], p2[0]);
	  WLZ_VTX_2_SUB(p2[2], p2[2], p2[0]);
	  WLZ_VTX_2_SUB(p2[3], p2[3], p2[1]);
	  p3[0] = nod[idx[0]]->pos;
	  p3[1] = nod[idx[1]]->pos;
	  p3[2] = nod[idx[2]]->pos;
	  d = (p2[1].vtX * p2[2].vtY) - (p2[2].vtX * p2[1].vtY);
#ifdef WLZ_CMESHINTERSECT_DEBUG
                (void )fprintf(stderr,
		               "WlzCMeshIntersectDom2D5 0 "
			       "0.0 0.0 %g %g %g %g "
			       "%g %g %g %g %g %g %g %g %g\n",
			       p2[1].vtX, p2[1].vtY,
			       p2[2].vtX, p2[2].vtY,
			       p3[0].vtX, p3[0].vtY, p3[0].vtZ,
			       p3[1].vtX, p3[1].vtY, p3[1].vtZ,
			       p3[2].vtX, p3[2].vtY, p3[2].vtZ);
#endif
	  if(fabs(d) < eps)
	  {
	    WlzDVertex3	q3;

	    if((fabs(p2[1].vtX - p2[2].vtX) < eps) &&
	       (fabs(p2[1].vtY - p2[2].vtY) < eps))
	    {
	      /* The 2D element has all vertices coincident. If the centroid
	       * of the 3D triangle is within the domain then set the value. */
		q3.vtX = (p3[0].vtX + p3[1].vtX + p3[2].vtX) / 3.0;
		q3.vtY = (p3[0].vtY + p3[1].vtY + p3[2].vtY) / 3.0;
		q3.vtZ = (p3[0].vtZ + p3[1].vtZ + p3[2].vtZ) / 3.0;
#ifdef WLZ_CMESHINTERSECT_DEBUG
                (void )fprintf(stderr,
		               "WlzCMeshIntersectDom2D5 1 %g %g %g %g %g\n",
		               p2[0].vtX, p2[0].vtY, q3.vtX, q3.vtY, q3.vtZ);
#endif
                if(WlzCMeshIntersectDomIsInside3D(sObj, q3, nrm, delta))
		{
		  WlzGreyValueGet(gVWSp, 0.0, p2[0].vtY, p2[0].vtX);
		  *((*(gVWSp->gPtr)).ubp) = 1;
		}
	    }
	    else
	    {
	      double	a,
	      		b;

	      /* The 2D element is reduced to a line segment. If voxels on
	       * the equivalent 3D line segment (from p3[0] to p3[2]) are
	       * inside the domain then set the value. */
	      if(p2[2].vtX > p2[2].vtY)
	      {
	        a = p2[2].vtY / p2[2].vtX;
		for(x2 = 0.0; x2 < p2[2].vtX + eps; x2 += 1.0)
		{
	          WlzDVertex2	q2;
	          WlzDVertex3	q3;

		  y2 = a * x2;
		  q2.vtX = x2;
		  q2.vtY = y2;
		  b = x2 / p2[2].vtX;
		  q3.vtX = p3[0].vtX + b * (p3[2].vtX - p3[0].vtX);
		  q3.vtY = p3[0].vtY + b * (p3[2].vtY - p3[0].vtY);
		  q3.vtZ = p3[0].vtZ + b * (p3[2].vtZ - p3[0].vtZ);
#ifdef WLZ_CMESHINTERSECT_DEBUG
                (void )fprintf(stderr,
		               "WlzCMeshIntersectDom2D5 2 %g %g %g %g %g\n",
		               p2[0].vtX, p2[0].vtY, q3.vtX, q3.vtY, q3.vtZ);
#endif
		  
		}
                if(WlzCMeshIntersectDomIsInside3D(sObj, q3, nrm, delta))
		{
		  WlzGreyValueGet(gVWSp, 0.0, p2[0].vtY, p2[0].vtX);
		  *((*(gVWSp->gPtr)).ubp) = 1;
		}
	      }
	      else
	      {
	        a = p2[2].vtX / p2[2].vtY;
		for(y2 = 0.0; y2 < p2[2].vtY + eps; y2 += 1.0)
		{
	          WlzDVertex2	q2;
	          WlzDVertex3	q3;

		  x2 = a * y2;
		  q2.vtX = x2;
		  q2.vtY = y2;
		  b = y2 / p2[2].vtY;
		  q3.vtX = p3[0].vtX + b * (p3[2].vtX - p3[0].vtX);
		  q3.vtY = p3[0].vtY + b * (p3[2].vtY - p3[0].vtY);
		  q3.vtZ = p3[0].vtZ + b * (p3[2].vtZ - p3[0].vtZ);
#ifdef WLZ_CMESHINTERSECT_DEBUG
                (void )fprintf(stderr,
		               "WlzCMeshIntersectDom2D5 2 %g %g %g %g %g\n",
		               p2[0].vtX, p2[0].vtY, q3.vtX, q3.vtY, q3.vtZ);
#endif
		}  
                if(WlzCMeshIntersectDomIsInside3D(sObj, q3, nrm, delta))
		{
		  WlzGreyValueGet(gVWSp, 0.0, p2[0].vtY, p2[0].vtX);
		  *((*(gVWSp->gPtr)).ubp) = 1;
		}
	      }
	    }
	  }
	  else
	  {
	    /* The 2D element is not degenerate. */
	    d = 1.0 / d;
	    /* Scan through all intervals of the scaled 2D element,
	     * transforming back to the 3D element and asking if each voxel
	     * is within the 3D domain. If it is set the grey value in the 2D
	     * scaled domain object. */
	    for(y2 = 0.0; y2 < p2[2].vtY + eps; y2 += 1.0)
	    {
	      double x0,
		     x1;

	      /* Compute points x0 and x1 on the current line segments. */
	      if(y2 < p2[1].vtY + eps)
	      {
		x0 = y2 * p2[1].vtX / p2[1].vtY;
		x1 = y2 * p2[2].vtX / p2[2].vtY;
	      }
	      else
	      {
		x0 = p2[1].vtX + ((y2 - p2[1].vtY) * p2[3].vtX / p2[3].vtY);
		x1 = y2 * p2[2].vtX / p2[2].vtY;
	      }
	      if(x0 > x1)
	      {
		double t;

		t = x0;
		x0 = x1;
		x1 = t;
	      }
	      for(x2 = x0; x2 < x1 + eps; x2 += 1.0)
	      {
		double		l[3]; /* The barycentric coordinates. */
		WlzDVertex2	q2;
		WlzDVertex3	q3;

		/* Map position q2 in the 2D trinagle to position q3 in the 3D
		 * triangle using linear interpolation based on barycentric
		 * coordinates. */
		q2.vtX = x2; 
		q2.vtY = y2;
		l[1] = d * ((p2[2].vtY * q2.vtX) - (p2[2].vtX * q2.vtY));
		l[2] = d * ((q2.vtY * p2[1].vtX) - (q2.vtX * p2[1].vtY));
		l[0] = 1.0 - (l[1] + l[2]);
		q3.vtX = l[0] * p3[0].vtX + l[1] * p3[1].vtX + l[2] * p3[2].vtX;
		q3.vtY = l[0] * p3[0].vtY + l[1] * p3[1].vtY + l[2] * p3[2].vtY;
		q3.vtZ = l[0] * p3[0].vtZ + l[1] * p3[1].vtZ + l[2] * p3[2].vtZ;
#ifdef WLZ_CMESHINTERSECT_DEBUG
                (void )fprintf(stderr,
		               "WlzCMeshIntersectDom2D5 3 %g %g %g %g %g\n",
		               q2.vtX, q2.vtY, q3.vtX, q3.vtY, q3.vtZ);
#endif
                if(WlzCMeshIntersectDomIsInside3D(sObj, q3, nrm, delta))
		{
		  WlzGreyValueGet(gVWSp, 0.0,
				  q2.vtY + p2[0].vtY, q2.vtX + p2[0].vtX);
		  *((*(gVWSp->gPtr)).ubp) = 1;
		}
	      }
	    }
	  }
	}
      }
    }
  }
  /* Threshold the flat object to get the 2D intersection domain for return. */
  if(errNum == WLZ_ERR_NONE)
  {
    WlzPixelV	v1;

    v1.type = WLZ_GREY_UBYTE;
    v1.v.ubv = 1;
    tObj[0] = WlzThreshold(fObj, v1, WLZ_THRESH_HIGH, &errNum);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    WlzValues	val;

    val.core = NULL;
    rObj = WlzMakeMain(WLZ_2D_DOMAINOBJ, tObj[0]->domain, val, NULL, NULL,
		       &errNum);
  }
  (void )WlzFreeObj(tObj[0]);
  (void )WlzFreeObj(fObj);
  if(dstErr != NULL)
  {
    *dstErr = errNum;
  }
  return(rObj);
}

/*!
* \return	Non-zero if the position is inside the domain.
* \ingroup	WlzMesh
* \brief	Checks to see if the given position is within the given 3D
* 		spatial domain. The delta value must be >= 0.0. If delta is
* 		> 0.0 then a check is made along in the direction of the given
* 		normal over a distance equal to delta.
* \param	sObj			Given 3D spatial domain object.
* \param	q			Given position.
* \param	nrm			Unit normal.
* \param	delta			Distance in normal direction.
*/
static int	WlzCMeshIntersectDomIsInside3D(WlzObject *sObj, WlzDVertex3 q,
					WlzDVertex3 nrm, double delta)
{
  int 		in = 0;

  if(WlzInsideDomain(sObj, q.vtZ, q.vtY, q.vtX, NULL))
  {
    in = 1;
  }
  else if(delta > WLZ_MESH_TOLERANCE)
  {
    double        d;
    WlzDVertex3   q0,
		  q1,
		  dNrm;

    d = delta;
    WLZ_VTX_3_SCALE(dNrm, nrm, delta);
    WLZ_VTX_3_SUB(q0, q, dNrm);
    WLZ_VTX_3_ADD(q1, q, dNrm);
    do
    {
      if((WlzInsideDomain(sObj, q0.vtZ, q0.vtY, q0.vtX, NULL)) ||
	 (WlzInsideDomain(sObj, q1.vtZ, q1.vtY, q1.vtX, NULL)))
      {
	in = 1;
	break;
      }
      d -=  1.0;
      WLZ_VTX_3_ADD(q0, q0, nrm);
      WLZ_VTX_3_SUB(q1, q1, nrm);
    } while(d > 0);
  }
  return(in);
}

/*!
* \return	Position of point on surface with minimum distance.
* \ingroup	WlzMesh
* \brief	Computes the position on a 2.5D conforming mesh which
* 		has the least distance to the given 3D spatial domain.
* \param	vObj			Voxel object with 3D spatial domain.
* \param	mObj			Object with the 2D5 mesh and
* 					displacements to a plane.
* \param	scale			Additional scale factor from mesh
* 					to spatial domain.
* \param	dstNPnt			Destination pointer for the number
* 					of points, may be NULL.
* \param	dstErr			Destination error pointer, may be NULL.
*/
WlzDVertex2	WlzCMeshClosePointDom2D5(WlzObject *vObj, WlzObject *mObj,
					 double scale, WlzErrorNum *dstErr)
{
  int		eIdx = -1;
  double	mDst = DBL_MAX;
  double	mL[3];
  WlzDVertex2	mPos2;
  WlzCMesh2D5	*mesh;
  WlzIterateWSpace *itWSp = NULL;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  mL[0] = mL[1] = mL[2] = 0.0;
  WLZ_VTX_2_SET(mPos2, 0.0, 0.0);
  if((vObj == NULL) || (mObj == NULL))
  {
    errNum = WLZ_ERR_OBJECT_NULL;
  }
  else if((vObj->type != WLZ_3D_DOMAINOBJ) || (mObj->type != WLZ_CMESH_2D5))
  {
    errNum = WLZ_ERR_OBJECT_TYPE;
  }
  else if((vObj->domain.core == NULL) || (mObj->domain.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else if((mObj->values.core == NULL))
  {
    errNum = WLZ_ERR_VALUES_NULL;
  }
  if(errNum == WLZ_ERR_NONE)
  {
    mesh = mObj->domain.cm2d5;
    itWSp = WlzIterateInit(vObj, WLZ_RASTERDIR_IPILIC, 0, &errNum);
  }
  /* For each voxel of the given 3D domain object. */
  while(errNum == WLZ_ERR_NONE)
  {
    errNum = WlzIterate(itWSp);
    if(errNum == WLZ_ERR_NONE)
    {
      int	nIdx;
      WlzDVertex3 vPos;

      WLZ_VTX_3_SET(vPos, itWSp->pos.vtX, itWSp->pos.vtY, itWSp->pos.vtZ);
      /* Find the mesh node that is closest to the voxel position, vPos. */
      nIdx = WlzCMeshClosestNod2D5(mesh, vPos);
      /* Find the point on the mesh surface (ie within an element) that
       * is closest to the point. */
      if(nIdx >= 0)
      {
	/* For each element which uses this node compute the minimum
	 * distance of the element to a vertex at the given position. */
	WlzCMeshNod2D5 *nod;

	nod = (WlzCMeshNod2D5 *)AlcVectorItemGet(mesh->res.nod.vec, nIdx);
	if(nod->idx >= 0)
	{
	  WlzCMeshEdgU2D5 *edu0,
			  *edu1;

	  edu0 = edu1 = nod->edu;
	  do
	  {
	    int		zT = 0;
	    double	d;
	    double	l[3];
	    WlzCMeshNod2D5 *nodes[3];

	    WlzCMeshElmGetNodes2D5(edu1->elm, nodes + 0, nodes + 1, nodes + 2);
	    d = WlzGeomTriangleVtxDistSq3D(NULL, &zT, NULL,
	    		       l + 0, l + 1, l + 2, vPos,
			       nodes[0]->pos, nodes[1]->pos, nodes[2]->pos);
	    if((zT == 0) && (d < mDst))
	    {
	      mDst = d;
	      eIdx = edu1->elm->idx;
	      mL[0] = l[0]; mL[1] = l[1]; mL[2] = l[2];
	    }
	    edu1 = edu1->nnxt;
	  } while(edu0 != edu1);
	}
      }
    }
  }
  WlzIterateWSpFree(itWSp);
  if(errNum == WLZ_ERR_EOO)
  {
    errNum = (eIdx < 0)? WLZ_ERR_DOMAIN_DATA: WLZ_ERR_NONE;
  }
  /* Interpolate the position in 2D using linear interpolation implimented
   * using barycentric coordinates. */
  if(errNum == WLZ_ERR_NONE)
  {
    double	*dsp;
    WlzCMeshElm2D5 *elm;
    WlzCMeshNod2D5 *nodes[3];
    WlzDVertex2	nP2[3];

    /* Get the nodes of the element containing the closest point on the
     * surface in 3D. */
    elm = (WlzCMeshElm2D5 *)AlcVectorItemGet(mesh->res.elm.vec, eIdx);
    WlzCMeshElmGetNodes2D5(elm, nodes + 0, nodes + 1, nodes + 2);
    /* Get the planar coordinates of the elements nodes. */
    dsp = (double *)WlzIndexedValueGet(mObj->values.x, nodes[0]->idx);
    nP2[0].vtX = nodes[0]->pos.vtX + dsp[0];
    nP2[0].vtY = nodes[0]->pos.vtY + dsp[1];
    dsp = (double *)WlzIndexedValueGet(mObj->values.x, nodes[1]->idx);
    nP2[1].vtX = nodes[1]->pos.vtX + dsp[0];
    nP2[1].vtY = nodes[1]->pos.vtY + dsp[1];
    dsp = (double *)WlzIndexedValueGet(mObj->values.x, nodes[2]->idx);
    nP2[2].vtX = nodes[2]->pos.vtX + dsp[0];
    nP2[2].vtY = nodes[2]->pos.vtY + dsp[1];
    /* Compute the planar position of the closest point using the
     * barycentric coordinates for linear interpolation. */
    WLZ_VTX_2_SUB(nP2[1], nP2[1], nP2[0]);
    WLZ_VTX_2_SUB(nP2[2], nP2[2], nP2[0]);
    mPos2.vtX = scale *
                (nP2[0].vtX + (mL[1] * nP2[1].vtX) + (mL[2] * nP2[2].vtX));
    mPos2.vtY = scale *
                (nP2[0].vtY + (mL[1] * nP2[1].vtY) + (mL[2] * nP2[2].vtY));
  }
  if(dstErr != NULL)
  {
    *dstErr = errNum;
  }
  return(mPos2);
}

/*!
* \return	Position of point in mesh with minimum distance.
* \ingroup	WlzMesh
* \brief	Computes the position in the 3D conforming mesh which
* 		has the least distance to the given position.
* \param	mObj			Object with the 3D mesh.
* \param	gPos			Given position.
* \param	dstCEI			Destination pointer for index to
* 					element that the closest point is
* 					within, may be NULL.
* \param	dstErr			Destination error pointer, may be NULL.
*/
WlzDVertex3	WlzCMeshClosestPoint3D(WlzObject *mObj, WlzDVertex3 gPos,
				       int *dstCEI, WlzErrorNum *dstErr)
{
  WlzDVertex3	rPos;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

#ifdef NEW_CODE_HACK
  /* First just look to see if a mesh element encloses the given point. */
  cEI = WlzCMeshElmJumpPos3D(mObj->mesh, gPos, NULL, &errNum);
  if(errNum == WLZ_ERR_NONE)
  {
    if(eEI > 0)
    {
      rPos = gPos;
    }
    else
    {
      /* TODO  HACK
	 Find closest grid cell to point.
	 While grid cell is empty spiral out.
	 */
    }
  }
  if((errNum == WLZ_ERR_NONE) && (dstCEI != NULL) && (cE != NULL))
  {
    dstCEI = cE->idx;
  }
#endif
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(rPos);
}


/*!
* \return	Signed sort indicator.
* \ingroup	WlzMesh
* \brief	Sort function for AlgHeapSortIdx() which results in WlzDVertex2
* 		vertices sorted by line then column coordinate.
* 		WLZ_MESH_TOLERANCE is used to compare vertex positions.
* \param	data			Used to pass the vertex array.
* \param	idx			The index array.
* \param	id0			First index.
* \param	id1			Second index.
*/
static int	WlzCMeshIntersectSortVtx2D(void *data, int *idx,
				           int id0, int id1)
{
  int		cmp = 0;
  WlzDVertex2	*v0,
  		*v1;
  const double 	eps = WLZ_MESH_TOLERANCE;
  
  v0 = (WlzDVertex2 *)data + *(idx + id0);
  v1 = (WlzDVertex2 *)data + *(idx + id1);
  if(v0->vtY > v1->vtY + eps)
  {
    cmp = 1;
  }
  else if(v0->vtY < v1->vtY - eps)
  {
    cmp = -1;
  }
  else
  {
    if(v0->vtX > v1->vtX + eps)
    {
      cmp = 1;
    }
    else if(v0->vtX < v1->vtX - eps)
    {
      cmp = -1;
    }
  }
  return(cmp);
}

#ifdef WLZ_UNUSED_FUNCTIONS
/*!
* \return	Zero if no intersection is possible otherwise non-zero.
* \ingroup	WlzGeometry
* \brief	Determines whether the axis aligned bounding box of the
* 		given object's domain and the given 3D triangle intersect.
* \param	dObj			Given object with domain.
* \param	p0			First vertex of triangle.
* \param	p1			Second vertex of triangle.
* \param	p2			Third vertex of triangle.
* \param	dstErr			Destination error pointer, may be NULL.
*/
static int	WlzIntersectDomAABBTri3D(WlzObject *dObj, WlzDVertex3 p0,
				     WlzDVertex3 p1, WlzDVertex3 p2,
				     WlzErrorNum *dstErr)
{
  int		isn = 0;

  if((dObj != NULL) && (dObj->type == WLZ_3D_DOMAINOBJ) &&
     (dObj->domain.core != NULL) &&
     (dObj->domain.core->type == WLZ_PLANEDOMAIN_DOMAIN))
  {
    WlzDVertex3	b0,
		b1;
    WlzPlaneDomain *pDom = NULL;
    
    pDom = dObj->domain.p;
    b0.vtX = pDom->kol1;
    b0.vtY = pDom->line1;
    b0.vtZ = pDom->plane1;
    b1.vtX = pDom->lastkl;
    b1.vtY = pDom->lastln;
    b1.vtZ = pDom->lastpl;
    isn = WlzGeomTriangleAABBIntersect3D(p0, p1, p2, b0, b1, 0);
  }
  return(isn);
}
#endif /* WLZ_UNUSED_FUNCTIONS */
