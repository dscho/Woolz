/************************************************************************
* Project:      Java Woolz
* Title:        WlzJavaArray3D.c
* Date:         January 1999
* Purpose:      3D array functions for the C side of Java Woolz.
* Copyright:	1997 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Maintenance:	Log changes below, with most recent at top of list.
* @author       Bill Hill (bill@hgu.mrc.ac.uk)
* @version 	MRC HGU %I%, %G%
************************************************************************/
#include <WlzJava.h>

/************************************************************************
* Function:	WlzJavaArray3DGet					*
* Returns:	jlong:			The value held in the given 	*
*					Java object.			*
* Purpose:	Returns a 3D array built from the given 3D java array.	*
* Global refs:	-							*
* Parameters:	JNIEnv *jEnv:		Given JNI environment ptr.	*
*		char *cObjName:		The Java woolz C object class	*
*					string.				*
*		char *jObjName:		The Java woolz Java object	*
*					class string.			*
*		char *jniObjName:	The Java woolz JNI object	*
*					class string.			*
*		int idrCnt:		Indirection count (ie 1 for *,	*
*					2 for **, ...).			*
*		int pKey:		Parameter key.			*
*		jarray jWArray:		The Java Woolz array.		*
*		WlzIVertex2 wArraySz:	The number of elements in the	*
*					Java Woolz array.		*
*		jboolean *isCpy:	Destination pointer for JNI	*
*					copy flag.			*
************************************************************************/
jlong 		WlzJavaArray3DGet(JNIEnv *jEnv,
				       char *cObjName,
				       char *jObjName,
				       char *jniObjName,
				       int idrCnt, int pKey,
				       jarray jWArray,
				       WlzIVertex3 wArraySz,
				       jboolean *isCpy)
{
  int		idY,
  		idZ,
		ok = 0;
  void		*bufJ;
  jobject	aryJ1D,
   		aryJ2D;
  void		***aryW3D = NULL;
  jlong		rtnVal = 0;

  if(jWArray &&
     (wArraySz.vtX > 0) && (wArraySz.vtY > 0) && (wArraySz.vtZ))
  {
    switch(pKey)
    {
      case WLZ_JPM_KEY_BYTE_ARY3:
	ok = AlcChar3Malloc((char ****)&aryW3D,
			    wArraySz.vtZ, wArraySz.vtY,
			    wArraySz.vtX) == ALC_ER_NONE;
	if(ok)
	{
	  idZ = 0; 
	  while(ok && (idZ < wArraySz.vtZ))
	  {
	    ok = (aryJ2D = (*jEnv)->GetObjectArrayElement(jEnv,
						(jobjectArray )jWArray,
						idZ)) != NULL;
	    if(ok)
	    {
	      idY = 0;
	      while(ok && (idY < wArraySz.vtY))
	      {
		ok = ((aryJ1D = (*jEnv)->GetObjectArrayElement(jEnv,
						(jobjectArray )aryJ2D,
						idY)) != NULL) &&
		     ((bufJ = (void *)(*jEnv)->GetByteArrayElements(jEnv,
		     				(jbyteArray )aryJ1D,
						isCpy)) != NULL);
		if(ok)
		{
		  (void )memcpy(*(*((WlzUByte ***)aryW3D + idZ) + idY), bufJ,
		  		wArraySz.vtX * sizeof(jbyte));
		  if(*isCpy)
		  {
		    (*jEnv)->ReleaseByteArrayElements(jEnv,
		    				(jbyteArray )aryJ1D,
						(jbyte *)bufJ, 0);
		    *isCpy = JNI_FALSE;
		  }
		}
	        ++idY;
	      }
	    }
	    ++idZ;
	  }
	}
	break;
      case WLZ_JPM_KEY_SHORT_ARY3:
	ok = AlcShort3Malloc((short ****)&aryW3D,
			     wArraySz.vtZ, wArraySz.vtY,
			     wArraySz.vtX) == ALC_ER_NONE;
	if(ok)
	{
	  idZ = 0; 
	  while(ok && (idZ < wArraySz.vtZ))
	  {
	    ok = (aryJ2D = (*jEnv)->GetObjectArrayElement(jEnv,
						(jobjectArray )jWArray,
						idZ)) != NULL;
	    if(ok)
	    {
	      idY = 0;
	      while(ok && (idY < wArraySz.vtY))
	      {
		ok = ((aryJ1D = (*jEnv)->GetObjectArrayElement(jEnv,
						(jobjectArray )aryJ2D,
						idY)) != NULL) &&
		     ((bufJ = (void *)(*jEnv)->GetShortArrayElements(jEnv,
		     				(jshortArray )aryJ1D,
						isCpy)) != NULL);
		if(ok)
		{
		  (void )memcpy(*(*((short ***)aryW3D + idZ) + idY), bufJ,
		  		wArraySz.vtX * sizeof(jshort));
		  if(*isCpy)
		  {
		    (*jEnv)->ReleaseShortArrayElements(jEnv,
		    				(jshortArray )aryJ1D,
						(jshort *)bufJ, 0);
		    *isCpy = JNI_FALSE;
		  }
		}
	        ++idY;
	      }
	    }
	    ++idZ;
	  }
	}
	break;
      case WLZ_JPM_KEY_INT_ARY3:
	ok = AlcInt3Malloc((int ****)&aryW3D,
			   wArraySz.vtZ, wArraySz.vtY,
			   wArraySz.vtX) == ALC_ER_NONE;
	if(ok)
	{
	  idZ = 0; 
	  while(ok && (idZ < wArraySz.vtZ))
	  {
	    ok = (aryJ2D = (*jEnv)->GetObjectArrayElement(jEnv,
						(jobjectArray )jWArray,
						idZ)) != NULL;
	    if(ok)
	    {
	      idY = 0;
	      while(ok && (idY < wArraySz.vtY))
	      {
		ok = ((aryJ1D = (*jEnv)->GetObjectArrayElement(jEnv,
						(jobjectArray )aryJ2D,
						idY)) != NULL) &&
		     ((bufJ = (void *)(*jEnv)->GetIntArrayElements(jEnv,
		     				(jintArray )aryJ1D,
						isCpy)) != NULL);
		if(ok)
		{
		  (void )memcpy(*(*((int ***)aryW3D + idZ) + idY), bufJ,
		  		wArraySz.vtX * sizeof(jint));
		  if(*isCpy)
		  {
		    (*jEnv)->ReleaseIntArrayElements(jEnv,
		    				(jintArray )aryJ1D,
						(jint *)bufJ, 0);
		    *isCpy = JNI_FALSE;
		  }
		}
	        ++idY;
	      }
	    }
	    ++idZ;
	  }
	}
	break;
      case WLZ_JPM_KEY_FLOAT_ARY3:
	ok = AlcFloat3Malloc((float ****)&aryW3D,
			     wArraySz.vtZ, wArraySz.vtY,
			     wArraySz.vtX) == ALC_ER_NONE;
	if(ok)
	{
	  idZ = 0; 
	  while(ok && (idZ < wArraySz.vtZ))
	  {
	    ok = (aryJ2D = (*jEnv)->GetObjectArrayElement(jEnv,
						(jobjectArray )jWArray,
						idZ)) != NULL;
	    if(ok)
	    {
	      idY = 0;
	      while(ok && (idY < wArraySz.vtY))
	      {
		ok = ((aryJ1D = (*jEnv)->GetObjectArrayElement(jEnv,
						(jobjectArray )aryJ2D,
						idY)) != NULL) &&
		     ((bufJ = (void *)(*jEnv)->GetFloatArrayElements(jEnv,
		     				(jfloatArray )aryJ1D,
						isCpy)) != NULL);
		if(ok)
		{
		  (void )memcpy(*(*((float ***)aryW3D + idZ) + idY), bufJ,
		  		wArraySz.vtX * sizeof(jfloat));
		  if(*isCpy)
		  {
		    (*jEnv)->ReleaseFloatArrayElements(jEnv,
		    				(jfloatArray )aryJ1D,
						(jfloat *)bufJ, 0);
		    *isCpy = JNI_FALSE;
		  }
		}
	        ++idY;
	      }
	    }
	    ++idZ;
	  }
	}
	break;
      case WLZ_JPM_KEY_DOUBLE_ARY3:
	ok = AlcDouble3Malloc((double ****)&aryW3D,
			      wArraySz.vtZ, wArraySz.vtY,
			      wArraySz.vtX) == ALC_ER_NONE;
	if(ok)
	{
	  idZ = 0; 
	  while(ok && (idZ < wArraySz.vtZ))
	  {
	    ok = (aryJ2D = (*jEnv)->GetObjectArrayElement(jEnv,
						(jobjectArray )jWArray,
						idZ)) != NULL;
	    if(ok)
	    {
	      idY = 0;
	      while(ok && (idY < wArraySz.vtY))
	      {
		ok = ((aryJ1D = (*jEnv)->GetObjectArrayElement(jEnv,
						(jobjectArray )aryJ2D,
						idY)) != NULL) &&
		     ((bufJ = (void *)(*jEnv)->GetDoubleArrayElements(jEnv,
		     				(jdoubleArray )aryJ1D,
						isCpy)) != NULL);
		if(ok)
		{
		  (void )memcpy(*(*((double ***)aryW3D + idZ) + idY), bufJ,
		  		wArraySz.vtX * sizeof(jdouble));
		  if(*isCpy)
		  {
		    (*jEnv)->ReleaseDoubleArrayElements(jEnv,
		    				(jdoubleArray )aryJ1D,
						(jdouble *)bufJ, 0);
		    *isCpy = JNI_FALSE;
		  }
		}
	        ++idY;
	      }
	    }
	    ++idZ;
	  }
	}
      default:
	break;
    }
    if(ok)
    {
      rtnVal = (jlong )aryW3D;
      *isCpy = JNI_TRUE;
    }
    else if(aryW3D)
    {
      Alc3Free(aryW3D);
    }
  }
  return(rtnVal);
}

/************************************************************************
* Function:	WlzJavaArray3DSet					*
* Returns:	void							*
* Purpose:	Sets the given value in a Java object for return to	*
*		the Java side of th JNI.				*
* Global refs:	-							*
* Parameters:	JNIEnv *jEnv:		Given JNI environment ptr.	*
*		jobjectArray dstJA:	Destination Java array.		*
*		char *cObjName:		The type of the value to set.	*
*               char *jObjName:         The Java woolz Java object      *
*                                       class string.                   *
*               char *jniObjName:       The Java woolz JNI object       *
*                                       class string.                   *
*		int idrCnt:		Indirection count (ie 1 for *,	*
*					2 for **, ...).			*
*		int pKey:		Parameter key.			*
*		void *aVal:		C array to set value from.	*
*		WlzIVertex3 aSz:	Size of the 3D array.		*
************************************************************************/
void		WlzJavaArray3DSet(JNIEnv *jEnv, jobjectArray dstJObj,
				char *cObjName,
				char *jObjName,
				char *jniObjName,
				int idrCnt, int pKey,
				void *aVal,
				WlzIVertex3 aSz)
{
  jobject	newJObj;

  if(aVal && (aSz.vtX > 0) && (aSz.vtY > 0) && (aSz.vtZ > 0))
  {
    switch(pKey)
    {
      case WLZ_JPM_KEY_INT_PTR1_ARY3:
	newJObj = WlzJavaArray3DWrap(jEnv, "int", "jint", "jintArray",
					  3, WLZ_JPM_KEY_INT_ARY3,
					  aVal, aSz);
	(*jEnv)->SetObjectArrayElement(jEnv, dstJObj, 0, newJObj);
	break;
      case WLZ_JPM_KEY_SHORT_PTR1_ARY3:
	newJObj = WlzJavaArray3DWrap(jEnv, "short", "jshort", "jshortArray",
					  3, WLZ_JPM_KEY_SHORT_ARY3,
					  aVal, aSz);
	(*jEnv)->SetObjectArrayElement(jEnv, dstJObj, 0, newJObj);
	break;
      case WLZ_JPM_KEY_BYTE_PTR1_ARY3:
	newJObj = WlzJavaArray3DWrap(jEnv, "char", "jbyte", "jbyteArray",
					  3, WLZ_JPM_KEY_BYTE_ARY3,
					  aVal, aSz);
	(*jEnv)->SetObjectArrayElement(jEnv, dstJObj, 0, newJObj);
	break;
      case WLZ_JPM_KEY_FLOAT_PTR1_ARY3:
	newJObj = WlzJavaArray3DWrap(jEnv, "float", "jfloat", "jfloatArray",
					  3, WLZ_JPM_KEY_FLOAT_ARY3,
					  aVal, aSz);
	(*jEnv)->SetObjectArrayElement(jEnv, dstJObj, 0, newJObj);
	break;
      case WLZ_JPM_KEY_DOUBLE_PTR1_ARY3:
	newJObj = WlzJavaArray3DWrap(jEnv, "double", "jdouble", "jdoubleArray",
					  3, WLZ_JPM_KEY_DOUBLE_ARY3,
					  aVal, aSz);
	(*jEnv)->SetObjectArrayElement(jEnv, dstJObj, 0, newJObj);
	break;
      default:
	break;
    }
  }
}

/************************************************************************
* Function:     WlzJavaArray3DWrap					*
* Returns:      jobject:                New Java object.                *
* Purpose:      Wraps up the given array as a new Java array.		*
* Global refs:  -                                                       *
* Parameters:   JNIEnv *jEnv:           Given JNI environment ptr.      *
*               char *cObjName:         The Java woolz object class     *
*                                       string.                         *
*               char *jObjName:         The Java woolz Java object      *
*                                       class string.                   *
*               char *jniObjName:       The Java woolz JNI object       *
*                                       class string.                   *
*               int idrCnt:             Indirection count (ie 1 for *,  *
*                                       2 for **, ...).                 *
*               int pKey:               Parameter key.                  *
*		void *aVal:		C array to set values from.	*
*		WlzIVertex3 aSz:	Size of the 3D array.		*
************************************************************************/
jobject		WlzJavaArray3DWrap(JNIEnv *jEnv,
				   char *cObjName,
				   char *jObjName,
				   char *jniObjName,
				   int idrCnt, int pKey,
				   void *aVal,
				   WlzIVertex3 aSz)
{
  int		idY,
		idZ,
		ok = 0;
  jclass	elmClass1D,
  		elmClass2D;
  jobject	jArray3D = NULL,
  		jArray2D = NULL,
		jArray1D = NULL,
  		rtnJObj = NULL;

#ifdef JWLZ_DEBUG
  int hack = 1;
  while(hack)
  {
    sleep(2);
  }
#endif /* JWLZ_DEBUG */
  if(aVal && (aSz.vtX > 0) && (aSz.vtY > 0) && (aSz.vtZ > 0))
  {
    switch(pKey)
    {
      case WLZ_JPM_KEY_INT_ARY3:
        jArray1D = (jobject )((*jEnv)->NewIntArray(jEnv, aSz.vtX));
	if(jArray1D)
	{
	  elmClass1D = (*jEnv)->GetObjectClass(jEnv, jArray1D);
	  jArray2D = (*jEnv)->NewObjectArray(jEnv, aSz.vtY, elmClass1D,
					     (jobject )NULL);
	}
	if(jArray2D)
	{
	  elmClass2D = (*jEnv)->GetObjectClass(jEnv, jArray2D);
	  jArray3D = (*jEnv)->NewObjectArray(jEnv, aSz.vtZ, elmClass2D,
	  				     (jobject )NULL);
	}
	if(jArray3D)
	{
	  idZ = 0;
	  while(jArray1D && jArray2D && (idZ < aSz.vtZ))
	  {
	    idY = 0;
	    while(jArray1D && (idY < aSz.vtY))
	    {
	      (*jEnv)->SetIntArrayRegion(jEnv, (jintArray )jArray1D,
	      			0, aSz.vtX,
				(jint *)*(*((int ***)aVal + idZ) + idY));
	      (*jEnv)->SetObjectArrayElement(jEnv, jArray2D, idY, jArray1D);
	      if(++idY < aSz.vtY)
	      {
		jArray1D = (jobject )((*jEnv)->NewIntArray(jEnv, aSz.vtX));
	      }
	    }
	    (*jEnv)->SetObjectArrayElement(jEnv, jArray3D, idZ, jArray2D);
	    if(++idZ < aSz.vtZ)
	    {
	      jArray2D = (*jEnv)->NewObjectArray(jEnv, aSz.vtY, elmClass1D,
	  				     	 (jobject )NULL);
	    }
	  }
	}
	ok = jArray3D && jArray2D && jArray1D;
	break;
      case WLZ_JPM_KEY_SHORT_ARY3:
        jArray1D = (jobject )((*jEnv)->NewShortArray(jEnv, aSz.vtX));
	if(jArray1D)
	{
	  elmClass1D = (*jEnv)->GetObjectClass(jEnv, jArray1D);
	  jArray2D = (*jEnv)->NewObjectArray(jEnv, aSz.vtY, elmClass1D,
					     (jobject )NULL);
	}
	if(jArray2D)
	{
	  elmClass2D = (*jEnv)->GetObjectClass(jEnv, jArray2D);
	  jArray3D = (*jEnv)->NewObjectArray(jEnv, aSz.vtZ, elmClass2D,
	  				     (jobject )NULL);
	}
	if(jArray3D)
	{
	  idZ = 0;
	  while(jArray1D && jArray2D && (idZ < aSz.vtZ))
	  {
	    idY = 0;
	    while(jArray1D && (idY < aSz.vtY))
	    {
	      (*jEnv)->SetShortArrayRegion(jEnv, (jshortArray )jArray1D,
	      			0, aSz.vtX,
				(jshort *)*(*((short ***)aVal + idZ) + idY));
	      (*jEnv)->SetObjectArrayElement(jEnv, jArray2D, idY, jArray1D);
	      if(++idY < aSz.vtY)
	      {
		jArray1D = (jobject )((*jEnv)->NewShortArray(jEnv, aSz.vtX));
	      }
	    }
	    (*jEnv)->SetObjectArrayElement(jEnv, jArray3D, idZ, jArray2D);
	    if(++idZ < aSz.vtZ)
	    {
	      jArray2D = (*jEnv)->NewObjectArray(jEnv, aSz.vtY, elmClass1D,
	  				     	 (jobject )NULL);
	    }
	  }
	}
	ok = jArray3D && jArray2D && jArray1D;
	break;
      case WLZ_JPM_KEY_BYTE_ARY3:
        jArray1D = (jobject )((*jEnv)->NewByteArray(jEnv, aSz.vtX));
	if(jArray1D)
	{
	  elmClass1D = (*jEnv)->GetObjectClass(jEnv, jArray1D);
	  jArray2D = (*jEnv)->NewObjectArray(jEnv, aSz.vtY, elmClass1D,
					     (jobject )NULL);
	}
	if(jArray2D)
	{
	  elmClass2D = (*jEnv)->GetObjectClass(jEnv, jArray2D);
	  jArray3D = (*jEnv)->NewObjectArray(jEnv, aSz.vtZ, elmClass2D,
	  				     (jobject )NULL);
	}
	if(jArray3D)
	{
	  idZ = 0;
	  while(jArray1D && jArray2D && (idZ < aSz.vtZ))
	  {
	    idY = 0;
	    while(jArray1D && (idY < aSz.vtY))
	    {
	      (*jEnv)->SetByteArrayRegion(jEnv, (jbyteArray )jArray1D,
	      			0, aSz.vtX,
				(jbyte *)*(*((WlzUByte ***)aVal + idZ) + idY));
	      (*jEnv)->SetObjectArrayElement(jEnv, jArray2D, idY, jArray1D);
	      if(++idY < aSz.vtY)
	      {
		jArray1D = (jobject )((*jEnv)->NewByteArray(jEnv, aSz.vtX));
	      }
	    }
	    (*jEnv)->SetObjectArrayElement(jEnv, jArray3D, idZ, jArray2D);
	    if(++idZ < aSz.vtZ)
	    {
	      jArray2D = (*jEnv)->NewObjectArray(jEnv, aSz.vtY, elmClass1D,
	  				     	 (jobject )NULL);
	    }
	  }
	}
	ok = jArray3D && jArray2D && jArray1D;
	break;
      case WLZ_JPM_KEY_FLOAT_ARY3:
        jArray1D = (jobject )((*jEnv)->NewFloatArray(jEnv, aSz.vtX));
	if(jArray1D)
	{
	  elmClass1D = (*jEnv)->GetObjectClass(jEnv, jArray1D);
	  jArray2D = (*jEnv)->NewObjectArray(jEnv, aSz.vtY, elmClass1D,
					     (jobject )NULL);
	}
	if(jArray2D)
	{
	  elmClass2D = (*jEnv)->GetObjectClass(jEnv, jArray2D);
	  jArray3D = (*jEnv)->NewObjectArray(jEnv, aSz.vtZ, elmClass2D,
	  				     (jobject )NULL);
	}
	if(jArray3D)
	{
	  idZ = 0;
	  while(jArray1D && jArray2D && (idZ < aSz.vtZ))
	  {
	    idY = 0;
	    while(jArray1D && (idY < aSz.vtY))
	    {
	      (*jEnv)->SetFloatArrayRegion(jEnv, (jfloatArray )jArray1D,
	      			0, aSz.vtX,
				(jfloat *)*(*((float ***)aVal + idZ) + idY));
	      (*jEnv)->SetObjectArrayElement(jEnv, jArray2D, idY, jArray1D);
	      if(++idY < aSz.vtY)
	      {
		jArray1D = (jobject )((*jEnv)->NewFloatArray(jEnv, aSz.vtX));
	      }
	    }
	    (*jEnv)->SetObjectArrayElement(jEnv, jArray3D, idZ, jArray2D);
	    if(++idZ < aSz.vtZ)
	    {
	      jArray2D = (*jEnv)->NewObjectArray(jEnv, aSz.vtY, elmClass1D,
	  				     	 (jobject )NULL);
	    }
	  }
	}
	ok = jArray3D && jArray2D && jArray1D;
	break;
      case WLZ_JPM_KEY_DOUBLE_ARY3:
        jArray1D = (jobject )((*jEnv)->NewDoubleArray(jEnv, aSz.vtX));
	if(jArray1D)
	{
	  elmClass1D = (*jEnv)->GetObjectClass(jEnv, jArray1D);
	  jArray2D = (*jEnv)->NewObjectArray(jEnv, aSz.vtY, elmClass1D,
					     (jobject )NULL);
	}
	if(jArray2D)
	{
	  elmClass2D = (*jEnv)->GetObjectClass(jEnv, jArray2D);
	  jArray3D = (*jEnv)->NewObjectArray(jEnv, aSz.vtZ, elmClass2D,
	  				     (jobject )NULL);
	}
	if(jArray3D)
	{
	  idZ = 0;
	  while(jArray1D && jArray2D && (idZ < aSz.vtZ))
	  {
	    idY = 0;
	    while(jArray1D && (idY < aSz.vtY))
	    {
	      (*jEnv)->SetDoubleArrayRegion(jEnv, (jdoubleArray )jArray1D,
	      			0, aSz.vtX,
				(jdouble *)*(*((double ***)aVal + idZ) + idY));
	      (*jEnv)->SetObjectArrayElement(jEnv, jArray2D, idY, jArray1D);
	      if(++idY < aSz.vtY)
	      {
		jArray1D = (jobject )((*jEnv)->NewDoubleArray(jEnv, aSz.vtX));
	      }
	    }
	    (*jEnv)->SetObjectArrayElement(jEnv, jArray3D, idZ, jArray2D);
	    if(++idZ < aSz.vtZ)
	    {
	      jArray2D = (*jEnv)->NewObjectArray(jEnv, aSz.vtY, elmClass1D,
	  				     	 (jobject )NULL);
	    }
	  }
	}
	ok = jArray3D && jArray2D && jArray1D;
	break;
      default:
	break;
    }
    if(ok)
    {
      rtnJObj = jArray3D;
    }
  }
  return(rtnJObj);
}

/************************************************************************
* Function:	WlzJavaArray3DFree					*
* Returns:	void							*
* Purpose:	Free's a temporary 3D array.				*
* Global refs:	-							*
* Parameters:	void *aDat:		Array data structure.		*
*		WlzIVertex3 aSz:	Array size.			*
*		int dSKey:		Data structure identification.	*
*		jboolean isCpy:		Copy flag for JNI functions.	*
************************************************************************/
void		WlzJavaArray3DFree(void *aDat, WlzIVertex3 aSz,
			      int dSKey, jboolean isCpy)
{
  if(isCpy && aDat && (aSz.vtX > 0) && (aSz.vtY > 0) && (aSz.vtZ > 0))
  {
    switch(dSKey)
    {
      case WLZ_JPM_KEY_INT_ARY3:   /* FALLTHROUGH */
      case WLZ_JPM_KEY_SHORT_ARY3: /* FALLTHROUGH */
      case WLZ_JPM_KEY_BYTE_ARY3:  /* FALLTHROUGH */
      case WLZ_JPM_KEY_FLOAT_ARY3: /* FALLTHROUGH */
      case WLZ_JPM_KEY_DOUBLE_ARY3:
        Alc3Free((void ***)aDat);
	break;
      default:
	break;
    }
  }
}
