
## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/gl/Makefile.am --
#
# Copyright (C) Olivier Chapuis, Nicolas Roussel
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

SUBDIRS = window texture scenegraph shading

noinst_LTLIBRARIES = libgl.la

libgl_la_SOURCES = glUtils.cxx  glUtils.H glIncludes.H

libgl_la_LIBADD = window/libglwindow.la texture/libgltexture.la scenegraph/libglscenegraph.la shading/libglshading.la

if HAVE_FREETYPE2
SUBDIRS += text
libgl_la_LIBADD += text/libgltext.la 
endif

pkgincludedir = $(includedir)/nucleo/gl

pkginclude_HEADERS = glUtils.H glIncludes.H
