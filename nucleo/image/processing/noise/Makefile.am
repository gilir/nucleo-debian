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

AM_CPPFLAGS = -I${top_srcdir}/ -I${top_builddir}/

noinst_LTLIBRARIES = libnoise.la

libnoise_la_SOURCES = PerlinNoise.cxx

# install headers

pkgincludedir = $(includedir)/nucleo/image/processing/noise

pkginclude_HEADERS = PerlinNoise.H