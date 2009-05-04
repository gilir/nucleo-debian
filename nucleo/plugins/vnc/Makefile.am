## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/plugins/vnc/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} @VNC_CPPFLAGS@

lib_LTLIBRARIES = libnPvnc.la
pkglib_LTLIBRARIES = nPvnc.la

# the module
nPvnc_la_SOURCES = vncImageSource.cxx
nPvnc_la_LDFLAGS = -module -avoid-version @VNC_LDFLAGS@
nPvnc_la_LIBADD = -lrfb

# the library
libnPvnc_la_SOURCES = vncImageSource.cxx
libnPvnc_la_LDFLAGS = -version 0:0:0 @VNC_LDFLAGS@
libnPvnc_la_LIBADD = -lrfb

libnPvncincludedir = $(includedir)/nucleo/plugins/vnc
libnPvncinclude_HEADERS = vncImageSource.H

noinst_SCRIPTS = plugin-descr

plugin-descr: nPvnc.la Makefile.am
	@. nPvnc.la  && echo $$dlname > plugin-descr ; \
	echo "   ImageSource::create vncImageSource_factory" >> plugin-descr ; \
	echo "      scheme=vnc" >> plugin-descr ; \
	touch ../plugin-list-timestamp
