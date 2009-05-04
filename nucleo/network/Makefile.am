## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/network/Makefile.am --
#
# Copyright (C) Olivier Chapuis, Nicolas Roussel
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

SUBDIRS = udp tcp http dnssd

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libnetwork.la
libnetwork_la_SOURCES = NetworkUtils.cxx
libnetwork_la_LIBADD = udp/libudp.la tcp/libtcp.la http/libhttp.la dnssd/libdnssd.la 

if HAVE_EXPAT
SUBDIRS += xmpp
libnetwork_la_LIBADD += xmpp/libxmpp.la 
endif

pkgincludedir = $(includedir)/nucleo/network
pkginclude_HEADERS = NetworkUtils.H
