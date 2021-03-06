#########################################################################
# Project:      Woolz
# Title:        Makefile
# Date:         August 1999
# Author:       Bill Hill
# Copyright:	1999 Medical Research Council, UK.
#		All rights reserved.
# Address:	MRC Human Genetics Unit,
#		Western General Hospital,
#		Edinburgh, EH4 2XU, UK.
# Purpose:      Makefile for Java/Woolz binding tests and demos.
# $Revision$
# $Id$
# Maintenance:	Log changes below, with most recent at top of list.
#########################################################################

# Include the configuration makefile (modify as required).
include			../../../../../../../../Makefile.conf

JAVASOURCES		= \
			  WlzAffineTransformLSq.java \
			  WlzArrayStats.java \
			  WlzArrayViewer.java \
			  WlzBndGreyInvert.java \
			  WlzBoundingBoxIntersection.java \
			  WlzBoundingBox.java \
			  WlzConstruct3D.java \
			  WlzDitherViewer.java \
			  WlzExtFFFacts.java \
			  WlzFacts.java \
			  WlzGetBackground.java \
			  WlzGreyStats.java \
			  WlzLabelTest.java \
			  WlzTest3DVtx.java \
			  WlzUnion.java \
			  WlzViewer.java

CLASSPATH		= .:../../../../../..
ifeq ($(UNIXTYPE), CYGWIN)
CLASSPATH		= ".;../../../../../.."
endif

JAVAFLAGS		= -classpath $(CLASSPATH)
JAVACFLAGS		= -classpath $(CLASSPATH)

CLASSES			= $(JAVASOURCES:%.java=%.class)

all:			classes

clean:
			$(RM) *.class

classes:		$(CLASSES)

demo:			$(CLASSES)
			@echo "Simple demos of a java-woolz object thats"
			@echo "in development. Look at the corresponding"
			@echo "java source file to see the code!"
			@echo
			@echo "WlzBoundingBox simply prints the bounding box"
			@echo "of the given woolz object:"
			$(JAVA) $(JAVAFLAGS) WlzBoundingBox t0.wlz
			@echo 
			@echo "WlzBoundingBoxIntersection prints the"
			@echo "intersection of the bounding box of the"
			@echo "given woolz object with a section plane:"
			$(JAVA) $(JAVAFLAGS) WlzBoundingBoxIntersection \
					     -d 20.0 -p 90.0 t0.wlz
			@echo 
			@echo "WlzDitherViewer throws up a window with the"
			@echo "second object dithered onto the first (the"
			@echo "window needs to be resized):"
			$(JAVA) $(JAVAFLAGS) WlzDitherViewer t0.wlz t1.wlz
			@echo 
			@echo "WlzFacts prints basic facts about the woolz"
			@echo "object:"
			$(JAVA) $(JAVAFLAGS) WlzFacts t0.wlz
			@echo 
			@echo "WlzGetBackground prints the objects background"
			@echo "value"
			$(JAVA) $(JAVAFLAGS) WlzGetBackground t0.wlz
			@echo 
			@echo "WlzGreyStats prints simple grey values stats"
			@echo "computed from the given object:"
			$(JAVA) $(JAVAFLAGS) WlzGreyStats t0.wlz
			@echo 
			@echo "WlzArrayStats calculates simple statistics on"
			@echo "the values in a 2D or 3D array created from"
			@echo "a Woolz object."
			$(JAVA) $(JAVAFLAGS) WlzArrayStats t0.wlz
			@echo 
			@echo "WlzArrayViewer throws up a window with the"
			@echo "the given object. (the window needs to be"
			@echo "resized):"
			$(JAVA) $(JAVAFLAGS) WlzArrayViewer t0.wlz
			@echo 
			@echo "WlzUnion computes the union of objects given"
			@echo "on the command line:"
			$(JAVA) $(JAVAFLAGS) WlzUnion -o out.wlz t0.wlz t1.wlz
			@echo 
			@echo "WlzViewer throws up a window with the"
			@echo "the given object, single left mouse click"
			@echo "shows grey value (the window needs to be"
			@echo "resized):"
			$(JAVA) $(JAVAFLAGS) WlzViewer t0.wlz
