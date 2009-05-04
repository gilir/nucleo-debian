## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/helpers/Makefile.am --
#
# Copyright (C) Nicolas Roussel
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libhelpers.la

libhelpers_la_SOURCES = OpenCV.H Phone.H Phone.cxx

pkgincludedir = $(includedir)/nucleo/helpers

pkginclude_HEADERS = OpenCV.H Phone.H

if DARWIN
libhelpers_la_SOURCES += AppleIRController.H AppleIRController.cxx
pkginclude_HEADERS += AppleIRController.H
endif
