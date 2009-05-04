## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/apps/videoServer/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
AM_LDFLAGS = -L${top_builddir}/nucleo -lNucleo

bin_PROGRAMS = videoServer

videoServer_SOURCES = main.cxx VideoServer.cxx \
	VideoService.cxx Notifier.cxx \
	FileStreamer.cxx VideoStreamer.cxx \
	FileStreamer.H Notifier.H Streamer.H \
	VideoServer.H      VideoService.H     VideoStreamer.H
