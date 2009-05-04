## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/image/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

SUBDIRS=encoding processing source sink

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libimage.la

libimage_la_SOURCES = Image.cxx Image.H

libimage_la_LIBADD = encoding/libencoding.la  processing/libprocessing.la \
	source/libsource.la sink/libsink.la

# install headers

pkgincludedir = $(includedir)/nucleo/image

pkginclude_HEADERS = Image.H RegionOfInterest.H ImageBridge.H


