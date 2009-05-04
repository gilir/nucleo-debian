## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/plugins/qt/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

QT4MOC    = @QT4MOC@

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} @QT_CPPFLAGS@

pkglib_LTLIBRARIES = nPqt.la

dist_nPqt_la_SOURCES = qtReactiveEngine.cxx qtFileKeeper.cxx qtTimeKeeper.cxx
nodist_nPqt_la_SOURCES = moc_qtReactiveEngine.cxx moc_qtTimeKeeper.cxx moc_qtFileKeeper.cxx
nPqt_la_LDFLAGS = -module -avoid-version @QT_LDFLAGS@
nPqt_la_LIBADD = @QT_LIBS@

moc_%.cxx: %.H
	$(QT4MOC) $(AM_CPPFLAGS) $< -o $@

libnPqtincludedir = $(includedir)/nucleo/plugins/qt
libnPqtinclude_HEADERS = qtReactiveEngine.H qtFileKeeper.H qtTimeKeeper.H

noinst_SCRIPTS = plugin-descr

plugin-descr: nPqt.la Makefile.am
	@. nPqt.la  && echo $$dlname > plugin-descr ; \
	echo "   ReactiveEngine::create qtReactiveEngine_factory" >> plugin-descr ; \
	echo "      type=qt" >> plugin-descr ; \
	touch ../plugin-list-timestamp

CLEANFILES = plugin-descr moc_qtFileKeeper.cxx moc_qtReactiveEngine.cxx moc_qtTimeKeeper.cxx
