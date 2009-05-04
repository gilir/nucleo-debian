## -*- Makefile -*- mode
## Process this file with automake to create Makefile.in
#
# tests/Makefile.am --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
AM_LDFLAGS = -L${top_builddir}/nucleo -lNucleo

noinst_PROGRAMS = \
	test-reducePath \
	test-StringUtils \
	test-URI test-URI-coding \
	test-HttpMessage test-httpServer \
	test-plugins \
	test-MemAlloc \
	test-speed \
	test-rdv test-udpplus \
	test-ReactiveEngine test-TimeKeeper test-SocketKeeper test-FileKeeper \
	test-UUID test-TimeStamp \
	test-Base64 test-MD5 \
     test-ByteOrder \
     test-Phone \
     test-Ping \
     test-UdpSocket

if HAVE_OPENGL
noinst_PROGRAMS += test-glWindow test-glTexture test-sgNode
if HAVE_FREETYPE2
noinst_PROGRAMS += test-glString
endif
endif

if DARWIN
noinst_PROGRAMS += test-AppleIRController
endif

if HAVE_EXPAT
noinst_PROGRAMS += test-XmlParser test-XmppConnection
endif

test_reducePath_SOURCES = test-reducePath.cxx

test_StringUtils_SOURCES = test-StringUtils.cxx

test_URI_SOURCES = test-URI.cxx
test_URI_coding_SOURCES = test-URI-coding.cxx

test_HttpMessage_SOURCES = test-HttpMessage.cxx

test_httpServer_SOURCES = test-httpServer.cxx

test_plugins_SOURCES = test-plugins.cxx

test_MemAlloc_SOURCES = test-MemAlloc.cxx

test_speed_SOURCES = test-speed.cxx

test_glWindow_SOURCES = test-glWindow.cxx

test_rdv_SOURCES = test-rdv.cxx

test_ReactiveEngine_SOURCES = test-ReactiveEngine.cxx
test_TimeKeeper_SOURCES = test-TimeKeeper.cxx
test_SocketKeeper_SOURCES = test-SocketKeeper.cxx
test_FileKeeper_SOURCES = test-FileKeeper.cxx

test_glTexture_SOURCES = test-glTexture.cxx
test_glString_SOURCES = test-glString.cxx

test_udpplus_SOURCES = test-udpplus.cxx

test_sgNode_SOURCES = test-sgNode.cxx

test_UUID_SOURCES = test-UUID.cxx
test_TimeStamp_SOURCES = test-TimeStamp.cxx

test_Base64_SOURCES = test-Base64.cxx
test_MD5_SOURCES = test-MD5.cxx

test_AppleIRController_SOURCES = test-AppleIRController.cxx

test_ByteOrder_SOURCES = test-ByteOrder.cxx

test_Phone_SOURCES = test-Phone.cxx

test_Ping_SOURCES = test-Ping.cxx

test_UdpSocket_SOURCES = test-UdpSocket.cxx

test_XmlParser_SOURCES = test-XmlParser.cxx
test_XmppConnection_SOURCES = test-XmppConnection.cxx
