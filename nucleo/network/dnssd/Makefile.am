## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/network/dnssd/Makefile.am --
#
# Copyright (C) Nicolas Roussel
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libdnssd.la

libdnssd_la_SOURCES = DNSServiceAnnouncer.cxx DNSServiceBrowser.cxx

pkgincludedir = $(includedir)/nucleo/network/dnssd

pkginclude_HEADERS = DNSService.H DNSServiceAnnouncer.H DNSServiceBrowser.H
