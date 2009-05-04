## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/image/processing/difference/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir}/ -I${top_builddir}/

noinst_LTLIBRARIES = libdifference.la

libdifference_la_SOURCES = \
	Difference.cxx		\
	MotionDetection.cxx	\
	SceneChangeDetector.cxx

# install headers

pkgincludedir = $(includedir)/nucleo/image/processing/difference

pkginclude_HEADERS = \
	Difference.H		\
	MotionDetection.H	\
	SceneChangeDetector.H