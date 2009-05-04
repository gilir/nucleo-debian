## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/network/http/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libhttp.la

libhttp_la_SOURCES = HttpMessage.cxx ServerPush.cxx

# install headers

pkgincludedir = $(includedir)/nucleo/network/http

pkginclude_HEADERS = HttpMessage.H  ServerPush.H