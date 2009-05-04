## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

SUBDIRS = nucleo apps demos data docs build-utils
if BUILD_TESTS
SUBDIRS += tests
endif

EXTRA_DIST = nucleo-config.in LICENSE
bin_SCRIPTS = nucleo-config

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = nucleo.pc

## ---------------------------------------------------------------------------
## Produce an rpm package using dist or from the given tarball
## (requires rpmbuild in your system)
#  Usage:
#    make rpm-dist
#    make release=1 rpm-dist
#    make rpm-this
#    make version=2.3.22 release=2 rpm-this
#    make rpm-dist cparams='--enable-multibyte --quiet' mparams='CFLAGS="-O2 -g"'

# automatical regeneration is missing for other dirs, so do it explicitly
rpm-regenerate:
	(cd build-utils/rpm && $(MAKE) $(AM_MAKEFLAGS) Makefile *.spec) || exit 1

rpm-dist: dist rpm-this

rpm-this: rpm-regenerate
	(cd build-utils/rpm && $(MAKE) $(AM_MAKEFLAGS) this) || exit 1
