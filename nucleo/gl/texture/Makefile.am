## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/gl/texture/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libgltexture.la

libgltexture_la_SOURCES = glTextureTile.cxx glTexture.cxx 

pkgincludedir = $(includedir)/nucleo/gl/texture

pkginclude_HEADERS = glTextureTile.H glTexture.H
