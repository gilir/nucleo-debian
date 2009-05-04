## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# demos/video/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
AM_LDFLAGS = -L${top_builddir}/nucleo -lNucleo

noinst_PROGRAMS = blender collage timeoverlay src2files files2dst

if HAVE_OPENGL
noinst_PROGRAMS += mini-videoProbe mini-mirrorSpace multiplex
if HAVE_FREETYPE2
noinst_PROGRAMS += simplegl
endif
endif

simplegl_SOURCES = simplegl.cxx
blender_SOURCES = blender.cxx
timeoverlay_SOURCES = timeoverlay.cxx
collage_SOURCES = collage.cxx
multiplex_SOURCES = multiplex.cxx
mini_videoProbe_SOURCES = mini-videoProbe.cxx
mini_mirrorSpace_SOURCES = mini-mirrorSpace.cxx
src2files_SOURCES = src2files.cxx
files2dst_SOURCES = files2dst.cxx
