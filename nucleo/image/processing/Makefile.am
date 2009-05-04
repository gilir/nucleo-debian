## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/image/processing/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

SUBDIRS=basic convolution difference chromakeying gamma noise

noinst_LTLIBRARIES = libprocessing.la

libprocessing_la_SOURCES = ImageFilter.cxx

libprocessing_la_LIBADD = basic/libbasic.la convolution/libconvolution.la \
	difference/libdifference.la chromakeying/libchromakeying.la \
	gamma/libgamma.la noise/libnoise.la

# install headers

pkgincludedir = $(includedir)/nucleo/image/processing

pkginclude_HEADERS = ImageFilter.H
