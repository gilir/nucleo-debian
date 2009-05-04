## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/network/tcp/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libtcp.la

libtcp_la_SOURCES = TcpUtils.cxx TcpServer.cxx TcpConnection.cxx Ping.cxx

pkgincludedir = $(includedir)/nucleo/network/tcp

pkginclude_HEADERS = TcpUtils.H TcpServer.H TcpConnection.H BufferedTcpConnection.H Ping.H
