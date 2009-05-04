#
# nucleo/plugins/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

SUBDIRS =
PLUGIN_DESCR =

if HAVE_VNC
SUBDIRS += vnc
PLUGIN_DESCR += vnc/plugin-descr 
endif

if HAVE_FFMPEG
SUBDIRS += ffmpeg
PLUGIN_DESCR += ffmpeg/plugin-descr 
endif

if HAVE_QT
SUBDIRS += qt
PLUGIN_DESCR += qt/plugin-descr 
endif

if HAVE_GD
SUBDIRS += gd
PLUGIN_DESCR += gd/plugin-descr 
endif

configdir = @NUCLEO_RESOURCES_DIR@
config_DATA = plugin-list

plugin-list: $(SUBDIR) $(PLUGIN_DESCR)
	@echo "# Nucleo plugin list, created on "`hostname`" ("`date`")" > plugin-list
	@echo "# Warning: indentation matters! Be careful if you edit this file..." >> plugin-list
	@if [ "$(SUBDIRS)" != "" ]; then \
        subdirs="$(SUBDIRS)" ; \
        for d in $$subdirs ; do \
		echo "" >> plugin-list ; \
		cat $$d/plugin-descr >> plugin-list ; \
	  done ; \
     fi

CLEANFILES = plugin-list plugin-list-timestamp
