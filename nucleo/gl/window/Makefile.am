## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/gl/window/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir}/ -I${top_builddir}/

if HAVE_GLX
GLX_S = glWindow_GLX.cxx
GLX_H = glWindow_GLX.H
endif

if HAVE_AGL
AGL_S = glWindow_AGL.cxx
AGL_H = glWindow_AGL.H
endif

noinst_LTLIBRARIES = libglwindow.la

# put the headers for make dist
libglwindow_la_SOURCES = \
	glWindow.cxx	\
	$(GLX_S)	\
	$(AGL_S)	\
	$(GLX_H)	\
	$(AGL_H)

# install headers

pkgincludedir = $(includedir)/nucleo/gl/window

pkginclude_HEADERS = \
	_events.H	\
	_extEvents.H	\
	glWindow.H	\
	keysym.H	\
	_macos2keysym.H	\
	$(GLX_H)	\
	$(AGL_H)
