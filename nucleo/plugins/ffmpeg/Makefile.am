## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# nucleo/plugins/ffmpeg/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} @FFMPEG_CPPFLAGS@ -D__STDC_CONSTANT_MACROS

pkglib_LTLIBRARIES = nPffmpeg.la

# the module
nPffmpeg_la_SOURCES = ffmpegImageSource.cxx ffmpegImageSource.H ffmpegImageSink.cxx ffmpegImageSink.H
nPffmpeg_la_LDFLAGS = -module -avoid-version @FFMPEG_LDFLAGS@
nPffmpeg_la_LIBADD  = @FFMPEG_LIBS@

#if DARWIN
#nPffmpeg_la_LDFLAGS += -Wl,-read_only_relocs,suppress
#endif

noinst_SCRIPTS = plugin-descr

plugin-descr: nPffmpeg.la Makefile.am
	@. nPffmpeg.la  && echo $$dlname > plugin-descr ; \
	echo "   ImageSource::create ffmpegImageSource_factory" >> plugin-descr ; \
	echo "      scheme=file" >> plugin-descr ; \
	echo "      scheme=ffmpeg-file" >> plugin-descr ; \
	echo "   ImageSink::create ffmpegImageSink_factory" >> plugin-descr ; \
	echo "      scheme=file" >> plugin-descr ; \
	echo "      scheme=ffmpeg-file" >> plugin-descr ; \
	echo "      scheme=mpegts-udp" >> plugin-descr ; \
	touch ../plugin-list-timestamp

CLEANFILES = plugin-descr
