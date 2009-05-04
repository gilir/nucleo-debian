## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/network/udp/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libudp.la

libudp_la_SOURCES = UdpReceiver.cxx UdpSender.cxx \
                    UdpPlus.cxx UdpPlusSender.cxx UdpPlusReceiver.cxx \
                    UdpSocket.cxx StunResolver.cxx StunResolverPrivate.cxx \
			StunResolverPrivate.H

pkgincludedir = $(includedir)/nucleo/network/udp

pkginclude_HEADERS = UdpReceiver.H UdpSender.H \
                     UdpPlus.H UdpPlusSender.H UdpPlusReceiver.H \
                     UdpSocket.H StunResolver.H
