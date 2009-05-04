## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# demos/opencv/faceFinder/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

OPENCV_PREFIX = @OPENCV_PREFIX@

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} @OPENCV_CPPFLAGS@ \
              -DHAARCASCADES=\"$(OPENCV_PREFIX)/share/opencv/haarcascades/\"

AM_LDFLAGS = -L${top_builddir}/nucleo -lNucleo @OPENCV_LDFLAGS@ @OPENCV_LIBS@

noinst_PROGRAMS = faceFinder

faceFinder_SOURCES = HaarClassifier.cxx faceFinder.cxx HaarClassifier.H
