## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# demos/misc/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
AM_LDFLAGS = -L${top_builddir}/nucleo -lNucleo

noinst_PROGRAMS = documentOpener disBonjour

if HAVE_OPENGL
noinst_PROGRAMS += paz
paz_SOURCES = paz.cxx
endif

if DARWIN
# this one doesn't compile on some Linux/Nvidia configs (why???)
if HAVE_OPENGL
noinst_PROGRAMS += multitexture
multitexture_SOURCES = multitexture.cxx
endif
endif

documentOpener_SOURCES = documentOpener.cxx
disBonjour_SOURCES = disBonjour.cxx

