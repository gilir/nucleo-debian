## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/image/processing/gamma/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir}/ -I${top_builddir}/

noinst_LTLIBRARIES = libgamma.la

libgamma_la_SOURCES = GammaCorrection.cxx

# install headers

pkgincludedir = $(includedir)/nucleo/image/processing/gamma

pkginclude_HEADERS = GammaCorrection.H