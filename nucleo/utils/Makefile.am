## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/utils/Makefile.am --
#
# Copyright (C) Olivier Chapuis, Nicolas Roussel
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libutils.la

libutils_la_SOURCES = \
	AppUtils.cxx \
	FileUtils.cxx \
	SignalUtils.cxx \
	StringUtils.cxx \
	TimeUtils.cxx \
	ConfigDict.cxx \
     Base64.cxx \
     MD5.cxx \
     ByteOrder.cxx

pkgincludedir = $(includedir)/nucleo/utils

pkginclude_HEADERS = \
	AppUtils.H \
	FileUtils.H \
	SignalUtils.H \
	StringUtils.H \
	TimeUtils.H \
	ConfigDict.H \
     Base64.H \
     MD5.H \
     ByteOrder.H
