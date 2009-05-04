## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# demos/vnc/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} @VNC_CPPFLAGS@

AM_LDFLAGS = -L${top_builddir}/nucleo/plugins/vnc -lnPvnc \
             -L${top_builddir}/nucleo -lNucleo \
             @VNC_LDFLAGS@ -lrfb

noinst_PROGRAMS = vnc-relay

vnc_relay_SOURCES = vnc-relay.cxx
