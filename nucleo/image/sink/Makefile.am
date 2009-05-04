## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/image/sink/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

noinst_LTLIBRARIES = libsink.la

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

libsink_la_SOURCES = ImageSink.cxx \
                     serverpushImageSink.cxx \
                     nudpImageSink.cxx nudppImageSink.cxx \
				 bufferedImageSink.cxx \
                     novImageSink.cxx

pkgincludedir = $(includedir)/nucleo/image/sink

pkginclude_HEADERS = ImageSink.H \
                     imagefileImageSink.H \
                     serverpushImageSink.H \
                     nudpImageSink.H  nudppImageSink.H \
                     nserverImageSink.H bufferedImageSink.H \
                     blackholeImageSink.H novImageSink.H

if HAVE_OPENGL
libsink_la_SOURCES += glwindowImageSink.cxx
pkginclude_HEADERS += glwindowImageSink.H
endif

if DARWIN
libsink_la_SOURCES += qtMovImageSink.cxx
pkginclude_HEADERS += qtMovImageSink.H
endif

# fix make dist (bug in automake?)
EXTRA_DIST =  qtMovImageSink.H
