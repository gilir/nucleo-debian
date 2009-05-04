## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/gl/scenegraph/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir}/ -I${top_builddir}/

noinst_LTLIBRARIES = libglscenegraph.la

libglscenegraph_la_SOURCES = \
	sgNode.cxx	\
	sgViewpoint.cxx

# install headers

pkgincludedir = $(includedir)/nucleo/gl/scenegraph

pkginclude_HEADERS = \
	sgClipper.H	\
	sgNode.H	\
	sgViewpoint.H
