## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/Makefile.am --
#
# Copyright (C) Nicolas Roussel
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} \
	-DNUCLEO_PLUGINS_DIR=\"$(NUCLEO_PLUGINS_DIR)\" \
	-DNUCLEO_RESOURCES_DIR=\"$(NUCLEO_RESOURCES_DIR)\"

SUBDIRS = core utils xml network image helpers plugins

lib_LTLIBRARIES = libNucleo.la

libNucleo_la_LIBADD = core/libcore.la utils/libutils.la xml/libxml.la image/libimage.la network/libnetwork.la helpers/libhelpers.la
libNucleo_la_SOURCES = nucleo.cxx

if HAVE_OPENGL
SUBDIRS += gl
libNucleo_la_LIBADD += gl/libgl.la
endif

# Shared library version info.  This is not the same as the package version
#
# -version-info CURRENT:REVISION:AGE
#
#   If either REVISION or AGE are omitted, they default to 0.  Also note
# that AGE must be less than or equal to the CURRENT interface number.
#
#  Here are a set of rules to help you update your library version
# information:
# 
#   1. Start with version information of `0:0:0' for each libtool library.
# 
#   2. Update the version information only immediately before a public
#      release of your software.  More frequent updates are unnecessary,
#      and only guarantee that the current interface number gets larger
#      faster.
# 
#   3. If the library source code has changed at all since the last
#      update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
# 
#   4. If any interfaces have been added, removed, or changed since the
#      last update, increment CURRENT, and set REVISION to 0.
# 
#   5. If any interfaces have been added since the last public release,
#      then increment AGE.
# 
#   6. If any interfaces have been removed since the last public release,
#      then set AGE to 0.

libNucleo_la_LDFLAGS = -version-info 0:0:0

libNucleoincludedir = $(includedir)/nucleo

# config.H should not ne installed
libNucleoinclude_HEADERS = nucleo.H

# make + install libNucleo (not the headers and the plugins)
ninstall: all
	$(MAKE) install-libLTLIBRARIES

# as above but also install the plugins
pninstall: all
	$(MAKE) install-exe
