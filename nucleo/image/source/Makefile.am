## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/image/source/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libsource.la

libsource_la_SOURCES = ImageSource.cxx  \
                       noiseImageSource.cxx \
                       serverpushImageSource.cxx nudpImageSource.cxx nudpcImageSource.cxx nudppImageSource.cxx \
                       vssImageSource.cxx nucImageSource.cxx novImageSource.cxx

pkgincludedir = $(includedir)/nucleo/image/source

pkginclude_HEADERS = ImageSource.H \
                     noiseImageSource.H \
                     imagefileImageSource.H \
                     serverpushImageSource.H nudpImageSource.H nudpcImageSource.H nudppImageSource.H \
                     vssImageSource.H nucImageSource.H novImageSource.H


if DARWIN
libsource_la_SOURCES += qtMovImageSource.cxx qtSgImageSource.cxx
pkginclude_HEADERS += qtMovImageSource.H qtSgImageSource.H
#libsource_la_SOURCES += oldQtSgImageSource.cxx 
#pkginclude_HEADERS += oldQtSgImageSource.H
endif

if LINUX
libsource_la_SOURCES += v4lImageSource.cxx
pkginclude_HEADERS += v4lImageSource.H
endif

# fix make dist (bug in automake?)
EXTRA_DIST =  qtMovImageSource.H qtSgImageSource.H v4lImageSource.H
