## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# apps/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

SUBDIRS = videoServer

AM_CPPFLAGS = -I${top_srcdir}/ -I${top_builddir}
AM_LDFLAGS = -L${top_builddir}/nucleo -lNucleo

bin_PROGRAMS = videoClient

videoClient_SOURCES = videoClient.cxx
