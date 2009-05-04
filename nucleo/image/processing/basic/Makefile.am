## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/image/processing/basic/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libbasic.la

libbasic_la_SOURCES = Resize.cxx Transform.cxx Paint.cxx

pkgincludedir = $(includedir)/nucleo/image/processing/basic

pkginclude_HEADERS = Resize.H Transform.H Paint.H