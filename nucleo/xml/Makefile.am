## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/xml/Makefile.am --
#
# Copyright (C) Olivier Chapuis, Nicolas Roussel
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

noinst_LTLIBRARIES = libxml.la

pkgincludedir = $(includedir)/nucleo/xml

libxml_la_SOURCES = XmlText.cxx XmlStructure.cxx
pkginclude_HEADERS = XmlText.H XmlStructure.H

if HAVE_EXPAT
libxml_la_SOURCES += XmlParser.cxx
pkginclude_HEADERS += XmlParser.H
endif
