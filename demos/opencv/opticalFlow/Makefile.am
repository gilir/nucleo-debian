## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# demos/opencv/opticalFlow/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} @OPENCV_CPPFLAGS@
AM_LDFLAGS = -L${top_builddir}/nucleo -lNucleo @OPENCV_LDFLAGS@ @OPENCV_LIBS@

noinst_PROGRAMS = miniflow flow

miniflow_SOURCES = OpticalFlow.cxx AutoFlow.cxx GridFlow.cxx OpticalFlowViewer.cxx miniflow.cxx OpticalFlow.H AutoFlow.H GridFlow.H OpticalFlowViewer.H
flow_SOURCES = OpticalFlow.cxx AutoFlow.cxx GridFlow.cxx OpticalFlowViewer.cxx flow.cxx OpticalFlow.H AutoFlow.H GridFlow.H OpticalFlowViewer.H
