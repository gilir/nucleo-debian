## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/image/encoding/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

noinst_LTLIBRARIES = libencoding.la

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

libencoding_la_SOURCES = Conversion.cxx YpCbCr420.cxx PAM.cxx

pkgincludedir = $(includedir)/nucleo/image/encoding

pkginclude_HEADERS = Conversion.H YpCbCr420.H PAM.H RGBAL.H

if DARWIN
libencoding_la_SOURCES += CoreGraphics.cxx
pkginclude_HEADERS += CoreGraphics.H
endif

if HAVE_LIBJPEG
libencoding_la_SOURCES += JPEG.cxx
pkginclude_HEADERS += JPEG.H
endif

if HAVE_LIBPNG
libencoding_la_SOURCES += PNGenc.cxx
pkginclude_HEADERS += PNGenc.H
endif
