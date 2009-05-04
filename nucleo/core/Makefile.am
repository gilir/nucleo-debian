## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/core/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

SUBDIRS = select

if DARWIN
SUBDIRS += corefoundation carbon cocoa
endif

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libcore.la

libcore_la_SOURCES = \
	TimeKeeper.cxx \
	FileKeeper.cxx \
	ReactiveEngine.cxx \
	ReactiveObject.cxx \
	URI.cxx \
	PluginManager.cxx \
	UUID.cxx \
	TimeStamp.cxx

pkgincludedir = $(includedir)/nucleo/core

pkginclude_HEADERS = \
	TimeKeeper.H \
	FileKeeper.H \
	ReactiveEngine.H \
	ReactiveObject.H \
	URI.H \
	PluginManager.H \
	UUID.H \
	TimeStamp.H

libcore_la_LIBADD = select/libselect.la
if DARWIN
libcore_la_LIBADD += corefoundation/libcorefoundation.la carbon/libcarbon.la cocoa/libcocoa.la
endif
