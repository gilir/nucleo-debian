## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# demos/opencv/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

SUBDIRS = faceFinder opticalFlow

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} @OPENCV_CPPFLAGS@
AM_LDFLAGS = -L${top_builddir}/nucleo -lNucleo @OPENCV_LDFLAGS@ @OPENCV_LIBS@

noinst_PROGRAMS = chessboardFinder morphodemo

chessboardFinder_SOURCES = chessboardFinder.cxx
morphodemo_SOURCES = morphodemo.cxx
