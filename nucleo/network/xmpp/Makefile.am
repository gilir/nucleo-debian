## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/network/xmpp/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libxmpp.la

libxmpp_la_SOURCES = XmppConnection.cxx
pkginclude_HEADERS = XmppConnection.H

pkgincludedir = $(includedir)/nucleo/network/xmpp
