## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/plugins/gd/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} @GD_CPPFLAGS@ -D__STDC_CONSTANT_MACROS

pkglib_LTLIBRARIES = nPgd.la

nPgd_la_SOURCES = AnimatedGifImageSink.cxx AnimatedGifImageSink.H
nPgd_la_LDFLAGS = -module -avoid-version @GD_LDFLAGS@
nPgd_la_LIBADD  = @GD_LIBS@

if DARWIN
nPgd_la_LDFLAGS += -Wl,-read_only_relocs,suppress
endif

noinst_SCRIPTS = plugin-descr

plugin-descr: nPgd.la Makefile.am
	@. nPgd.la  && echo $$dlname > plugin-descr ; \
	echo "   ImageSink::create AnimatedGifImageSink_factory" >> plugin-descr ; \
	echo "      scheme=animated-gif" >> plugin-descr ; \
	touch ../plugin-list-timestamp

CLEANFILES = plugin-descr
