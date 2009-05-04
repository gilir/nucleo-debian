#
# configure.ac --
#
# Copyright (C) Nicolas Roussel, Olivier Chapuis
#
# See the file LICENSE for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

# --------------------------------------------------------

AC_PREREQ(2.58)
AC_INIT(nucleo, 0.7.3, roussel@lri.fr)
AC_CONFIG_SRCDIR([nucleo/core/ReactiveObject.H])
AC_CONFIG_AUX_DIR(build-utils)

AM_INIT_AUTOMAKE([dist-bzip2])
AM_CONFIG_HEADER([nucleo/config.H])

AC_LANG(C++)
AC_PROG_CXX
AC_PROG_OBJC
_AM_DEPENDENCIES([OBJC])

AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_PATH_TOOL([PKG_CONFIG], [pkg-config])
AC_DEFUN([MY_PKG_CONFIG_CHECK], [
  AC_MSG_CHECKING([for $1 using pkg-config])
  $2=0
  if test -n "$PKG_CONFIG" && AC_RUN_LOG([$PKG_CONFIG --exists $1]); then
    $2=1
    if test -z "$3"; then
      CPPFLAGS="$CPPFLAGS `$PKG_CONFIG --cflags-only-I $1`"
      LDFLAGS="$LDFLAGS `$PKG_CONFIG --libs-only-L $1`"
      LIBS="$LIBS `$PKG_CONFIG --libs-only-l $1`"
    else
      $3_CPPFLAGS=`$PKG_CONFIG --cflags-only-I $1`
      $3_LDFLAGS=`$PKG_CONFIG --libs-only-L $1`
      $3_LIBS=`$PKG_CONFIG --libs-only-l $1`
    fi
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
])

# --------------------------------------------------------

TARGET="unknown"
case "$host" in
  *darwin*) TARGET=DARWIN ;;
   *linux*) TARGET=LINUX ;;
  *cygwin*) TARGET=CYGWIN ;;
esac

# Use the name in the comment in source files
AM_CONDITIONAL(DARWIN, test x$TARGET = xDARWIN)  # __APPLE__
AM_CONDITIONAL(LINUX, test x$TARGET = xLINUX)    # __linux__
AM_CONDITIONAL(CYGWIN, test x$TARGET = xCYGWIN)  # __CYGWIN__

SIZEOF_POINTER_GT_SIZEOF_UINT=0

AC_CHECK_SIZEOF(void *)
AC_CHECK_SIZEOF(unsigned)
echo "debug unsigned int size: $ac_cv_sizeof_unsigned"
if test "$ac_cv_sizeof_void_p" -gt "$ac_cv_sizeof_unsigned"; then
  echo "The type void * is bigger than unsigned int."
  SIZEOF_POINTER_GT_SIZEOF_INT=1
  AC_DEFINE(SIZEOF_POINTER_GT_SIZEOF_UINT,1,[Sizeof(void *) > sizeof(unsigned int)])
fi

AM_CONDITIONAL(SIZEOF_POINTER_GT_SIZEOF_UINT,[test $SIZEOF_POINTER_GT_SIZEOF_UINT = 1])

AC_CHECK_HEADERS([tr1/unordered_map hash_map ext/hash_map])
  	
# --------------------------------------------------------

# This is where the plugins are installed (used in AppUtils.cxx)
NUCLEO_PLUGINS_DIR='${pkglibdir}'
AC_SUBST(NUCLEO_PLUGINS_DIR)

# This is where the "plugin-list" file is installed. This directory
# also contains the Bitstream Vera fonts, in a "fonts" subdirectory.
# This is used in AppUtils.cxx.
NUCLEO_RESOURCES_DIR='${pkgdatadir}'
AC_SUBST(NUCLEO_RESOURCES_DIR)

# --------------------------------------------------------
# OpenGL stuff (AGL, GLX, etc.)

HAVE_OPENGL=0
HAVE_AGL=0
HAVE_GLX=0
HAVE_XINPUT=0

problem_opengl=""
with_opengl="no"

if test x$TARGET = xDARWIN; then
    CPPFLAGS="$CPPFLAGS -I/sw/include"
    CXXFLAGS="$CXXFLAGS -Wall -pipe -O3"
    LDFLAGS="$LDFLAGS -L/sw/lib"
    # ---------------------------------
    # FIXME: -headerpad_max_install_names is 32-bit only, not tested on 64-bit
    LDFLAGS="$LDFLAGS -Wl,-headerpad_max_install_names" # -flat_namespace -bind_at_load
    # ---------------------------------
    if test "$with_x" = yes; then
      LOOK_FOR_GLX=yes
    else
      LOOK_FOR_GLX=no
      LIBS="$LIBS -framework AGL -framework OpenGL"
	 # --- Leopard fix -----------------
	 LIBS="$LIBS -dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib -framework Carbon"
	 # ---------------------------------
      HAVE_AGL=1 ; AC_DEFINE(HAVE_AGL,1,[AGL-based OpenGL implementation])
      HAVE_OPENGL=1 ; AC_DEFINE(HAVE_OPENGL,1,[OpenGL support])
      with_opengl="AGL"
    fi
    LIBS="$LIBS -framework QuickTime -framework QuartzCore -framework ApplicationServices -framework Accelerate"
elif test x$TARGET = CYGWIN; then
    CPPFLAGS="$CPPFLAGS -I/usr/include"
    LDFLAGS="$LDFLAGS -L/usr/lib"
    LOOK_FOR_GLX=yes
else
    LOOK_FOR_GLX=yes
fi

if test "$LOOK_FOR_GLX" = yes; then
    glx_saved_LIBS=$LIBS
    glx_saved_CPPFLAGS=$CPPFLAGS
    AC_PATH_XTRA
    LIBS="$LIBS $X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS"
    CPPFLAGS="$CPPFLAGS $X_CFLAGS"
    AC_CHECK_HEADERS([GL/gl.h], [], [problem_opengl=" (gl.h not found)"])
    AC_CHECK_HEADERS([GL/glext.h GL/glu.h GL/glx.h], [],
                     [problem_opengl=" (glext.h, glu.h or glx.h not found)"],
                     [#include <GL/gl.h>]) 
    AC_CHECK_LIB(pthread,pthread_create)
    AC_CHECK_LIB(GL,glFlush,[],[problem_opengl=" (libGL not found)"])
    AC_CHECK_LIB(GLU,gluUnProject,[],[problem_opengl=" (libGLU not found)"])
    if test "$have_x" = yes && test "$problem_opengl" = ""; then
      HAVE_GLX=1 ; AC_DEFINE(HAVE_GLX,1,[GLX-based OpenGL implementation])
      HAVE_OPENGL=1 ; AC_DEFINE(HAVE_OPENGL,1,[OpenGL support])
      with_opengl="GLX"
      AC_CHECK_LIB(Xi, XListInputDevices, [LIBS="$LIBS -lXi" ; HAVE_XINPUT=1 ;
                   AC_DEFINE(HAVE_XINPUT,1,[XInput support])])
      if test $HAVE_XINPUT = 1; then
        with_opengl="GLX + Xinput"
      fi
      problem_ProcAdd="glXGetProcAddress and glXGetProcAddress not found"
      AC_CHECK_LIB(GL,glXGetProcAddress,[problem_ProcAdd=""; 
                   with_opengl="$with_opengl glXGetProcAddress";
                   AC_DEFINE(HAVE_glXGetProcAddress,1,
                             [have glXGetProcAddress])],[])
      if test "$problem_ProcAdd" != ""; then
	AC_CHECK_LIB(GL,glXGetProcAddressARB,[problem_ProcAdd=""; 
                   with_opengl="$with_opengl glXGetProcAddressARB";
                   AC_DEFINE(HAVE_glXGetProcAddressARB,1,
                             [have glXGetProcAddressARB])],[])
      fi
      if test "$problem_ProcAdd" != ""; then
        with_opengl="$with_opengl (no glXGetProcAddress* func)"
      fi
    else
      with_opengl="no"
      LIBS=$glx_saved_LIBS
      CPPFLAGS=$glx_saved_CPPFLAGS
    fi
fi

AC_SUBST(HAVE_OPENGL)
AM_CONDITIONAL(HAVE_OPENGL,[test $HAVE_OPENGL = 1])
AC_SUBST(HAVE_AGL)
AM_CONDITIONAL(HAVE_AGL,[test $HAVE_AGL = 1])
AC_SUBST(HAVE_GLX)
AM_CONDITIONAL(HAVE_GLX,[test $HAVE_GLX = 1])
AC_SUBST(HAVE_XINPUT)
AM_CONDITIONAL(HAVE_XINPUT,[test $HAVE_XINPUT = 1])

# --------------------------------------------------------
# Other optional stuff that might get into libNucleo

# --- CoreGraphics or (libjpeg, libpng, libexif)

if test x$TARGET = xDARWIN; then
  HAVE_JPEG=1
  HAVE_PNG=1
  HAVE_EXIF=1

  problem_jpg=" (using CoreGraphics)"
  with_jpg="yes"
  AC_DEFINE_UNQUOTED(HAVE_LIBJPEG,0,[libjpeg support])
  AM_CONDITIONAL(HAVE_LIBJPEG,0)
  AC_DEFINE_UNQUOTED(HAVE_JPEG,1,[JPEG support])
  AM_CONDITIONAL(HAVE_JPEG,1)

  problem_png=" (using CoreGraphics)"
  with_png="yes"
  AC_DEFINE_UNQUOTED(HAVE_LIBPNG,0,[libpng support])
  AM_CONDITIONAL(HAVE_LIBPNG,0)
  AC_DEFINE_UNQUOTED(HAVE_PNG,1,[PNG support])
  AM_CONDITIONAL(HAVE_PNG,1)

  problem_exif=" (using CoreGraphics)"
  with_exif="yes"
  AC_DEFINE_UNQUOTED(HAVE_LIBEXIF,0,[libexif support])
  AM_CONDITIONAL(HAVE_LIBEXIF,0)
  AC_DEFINE_UNQUOTED(HAVE_EXIF,1,[EXIF support])
  AM_CONDITIONAL(HAVE_EXIF,1)
else
  problem_jpg=" (using libjpeg)"
  with_jpg="yes"
  HAVE_LIBJPEG=1
  AC_CHECK_LIB(jpeg,jpeg_start_decompress, [],
     [HAVE_LIBJPEG=0; with_jpg="no"; problem_jpg=" (libjpeg not found)"])
  AC_CHECK_HEADERS([jpeglib.h], [],
     [HAVE_LIBJPEG=0; with_jpg="no"; problem_jpg=" (jpeglib.h not found)"])
  AC_SUBST(HAVE_LIBJPEG)
  AC_DEFINE_UNQUOTED(HAVE_LIBJPEG,$HAVE_LIBJPEG,[libjpeg support])
  AM_CONDITIONAL(HAVE_LIBJPEG,[test "x$HAVE_LIBJPEG" = "x1"])
  AC_DEFINE_UNQUOTED(HAVE_JPEG,$HAVE_LIBJPEG,[JPEG support])
  AM_CONDITIONAL(HAVE_JPEG,[test "x$HAVE_LIBJPEG" = "x1"])

  problem_png=" (using libpng)"
  with_png="yes"
  MY_PKG_CONFIG_CHECK([libpng], HAVE_LIBPNG)
  if test $HAVE_LIBPNG = 0; then
    HAVE_LIBPNG=1
    AC_CHECK_LIB(z, inflate, [],
      [HAVE_LIBPNG=0; with_png="no"; problem_png=" (zlib not found)"])
    AC_CHECK_LIB(png,png_sig_cmp, [],
      [HAVE_LIBPNG=0; with_png="no"; problem_png=" (libpng not found)"])
    AC_CHECK_HEADERS([png.h], [],
      [HAVE_LIBPNG=0; with_png="no"; problem_png=" (png.h not found)"])
  fi
  AC_SUBST(HAVE_LIBPNG)
  AC_DEFINE_UNQUOTED(HAVE_LIBPNG,$HAVE_LIBPNG,[libpng support])
  AM_CONDITIONAL(HAVE_LIBPNG,[test "x$HAVE_LIBPNG" = "x1"])
  AC_DEFINE_UNQUOTED(HAVE_PNG,$HAVE_LIBPNG,[PNG support])
  AM_CONDITIONAL(HAVE_PNG,[test "x$HAVE_LIBPNG" = "x1"])

  problem_exif=" (using libexif)"
  with_exif="yes"
  MY_PKG_CONFIG_CHECK([libexif], HAVE_LIBEXIF)
  AC_SUBST(HAVE_LIBEXIF)
  AC_DEFINE_UNQUOTED(HAVE_LIBEXIF,$HAVE_LIBEXIF,[libexif support])
  AM_CONDITIONAL(HAVE_LIBEXIF,[test "x$HAVE_LIBEXIF" = "x1"])
  if test $HAVE_LIBEXIF = 0; then
    problem_exif=" (libexif not found by pkg-config)"
    with_exif="no"
  fi
  AC_DEFINE_UNQUOTED(HAVE_EXIF,$HAVE_LIBEXIF,[EXIF support])
  AM_CONDITIONAL(HAVE_EXIF,[test "x$HAVE_LIBEXIF" = "x1"])

  HAVE_JPEG=$HAVE_LIBJPEG
  HAVE_PNG=$HAVE_LIBPNG
  HAVE_EXIF=$HAVE_LIBEXIF
fi

AC_SUBST(HAVE_JPEG)
AC_SUBST(HAVE_PNG)
AC_SUBST(HAVE_EXIF)

# --- Freetype 2

AC_ARG_WITH(freetype2,
  AS_HELP_STRING([--with-freetype2], [build with Freetype2 support (default=autodetect)]),
  with_freetype2=$withval,
  with_freetype2="autodetect")

if test "$with_freetype2" = "autodetect" || test "$with_freetype2" = "yes"; then
  MY_PKG_CONFIG_CHECK([freetype2], HAVE_FREETYPE2)
else
  HAVE_FREETYPE2=0
fi
if test "$HAVE_FREETYPE2" = "0" && test "$with_freetype2" = "yes"; then
  AC_MSG_ERROR("could not find Freetype2")
fi
AC_SUBST(HAVE_FREETYPE2)
AC_DEFINE_UNQUOTED(HAVE_FREETYPE2,$HAVE_FREETYPE2,[Freetype2 support])
AM_CONDITIONAL(HAVE_FREETYPE2,[test "x$HAVE_FREETYPE2" = "x1"])
if test $HAVE_FREETYPE2 = 0; then
  test "$with_freetype" = "autodetect" && problem_freetype2=" (Freetype2 not found by pkg-config)"
  with_freetype2="no"
else
  with_freetype2="yes"
fi

# --- dns-sd

AC_ARG_WITH(dns_sd,
  AS_HELP_STRING([--with-dns-sd], [build with DNS-SD support (default=autodetect)]),
  with_dns_sd=$withval,
  with_dns_sd="autodetect")

problem_dns_sd=""
if test "$with_dns_sd" = "autodetect" || test "$with_dns_sd" = "yes"; then
  HAVE_DNS_SD=1
  with_dns_sd="yes"
  if test x$TARGET != xDARWIN; then
    AC_CHECK_LIB(dns_sd, DNSServiceRegister, [],
      [HAVE_DNS_SD=0; with_dns_sd="no"; problem_dns_sd=" (dns_sd library not found)"])
  fi
  AC_CHECK_HEADERS([dns_sd.h], [],
    [HAVE_DNS_SD=0; with_dns_sd="no"; problem_dns_sd=" (dns_sd.h not found)"])
else
  HAVE_DNS_SD=0
fi
AC_SUBST(HAVE_DNS_SD)
AC_DEFINE_UNQUOTED(HAVE_DNS_SD,$HAVE_DNS_SD,[DNS_SD support])
AM_CONDITIONAL(HAVE_DNS_SD,[test "x$HAVE_DNS_SD" = "x1"])

# ---expat

AC_ARG_WITH(expat,
  AS_HELP_STRING([--with-expat], [build with expat support (default=autodetect)]),
  with_expat=$withval,
  with_expat="autodetect")

problem_expat=""
if test "$with_expat" != "no"; then
  HAVE_EXPAT=1
  with_expat="yes"
  AC_CHECK_LIB(expat, XML_GetFeatureList, [],
    [HAVE_EXPAT=0; with_expat="no"; problem_expat=" (expat library not found)"])
  AC_CHECK_HEADERS([expat.h], [],
    [HAVE_EXPAT=0; with_expat="no"; problem_expat=" (expat.h not found)"])
else
  HAVE_EXPAT=0
fi
if test "$with_expat" = "yes" && test "$HAVE_EXPAT" = "0" ; then
  AC_MSG_ERROR("could not find expat")
fi
AC_SUBST(HAVE_EXPAT)
AC_DEFINE_UNQUOTED(HAVE_EXPAT,$HAVE_EXPAT,[EXPAT support])
AM_CONDITIONAL(HAVE_EXPAT,[test "x$HAVE_EXPAT" = "x1"])

# --- GnuTLS

AC_ARG_WITH(gnutls,
  AS_HELP_STRING([--with-gnutls], [build with GnuTLS support (default=autodetect)]),
  with_gnutls=$withval,
  with_gnutls="autodetect")

if test "$with_gnutls" = "autodetect" || test "$with_gnutls" = "yes"; then
  MY_PKG_CONFIG_CHECK([gnutls], HAVE_GNUTLS)
else
  HAVE_GNUTLS=0
fi
if test "$HAVE_GNUTLS" = "0" && test "$with_gnutls" = "yes"; then
  AC_MSG_ERROR("could not find GnuTLS")
fi
AC_SUBST(HAVE_GNUTLS)
AC_DEFINE_UNQUOTED(HAVE_GNUTLS,$HAVE_GNUTLS,[GnuTLS support])
AM_CONDITIONAL(HAVE_GNUTLS,[test "x$HAVE_GNUTLS" = "x1"])
if test $HAVE_GNUTLS = 0; then
  test "$with_gnutls" = "autodetect" && problem_gnutls=" (GnuTLS not found by pkg-config)"
  with_gnutls="no"
else
  with_gnutls="yes"
fi

# --- dl

AC_CHECK_HEADERS([dlfcn.h dl.h])
AC_CHECK_FUNCS(dlopen, [], [AC_CHECK_LIB(dl, dlopen, LIBS="$LIBS -ldl")])

# --------------------------------------------------------
# Plugins and demos (outside libNucleo)

# --- FFmpeg

AC_ARG_WITH(ffmpeg,
  AS_HELP_STRING([--with-ffmpeg], [build FFmpeg plugin (default=autodetect)]),
  with_ffmpeg=$withval,
  with_ffmpeg="autodetect")

if test "$with_ffmpeg" = "autodetect" || test "$with_ffmpeg" = "yes"; then
  MY_PKG_CONFIG_CHECK([libavformat], HAVE_FFMPEG, FFMPEG)
else
  HAVE_FFMPEG=0
fi
if test "$HAVE_FFMPEG" = "0" && test "$with_ffmpeg" = "yes"; then
  AC_MSG_ERROR("could not find ffmpeg")
fi
AC_SUBST(HAVE_FFMPEG)
AC_SUBST(FFMPEG_LDFLAGS)
AC_SUBST(FFMPEG_CPPFLAGS)
AC_SUBST(FFMPEG_LIBS)
AC_DEFINE_UNQUOTED(HAVE_FFMPEG,$HAVE_FFMPEG,[FFmpeg support])
AM_CONDITIONAL(HAVE_FFMPEG,[test "x$HAVE_FFMPEG" = "x1"])
if test $HAVE_FFMPEG = 0; then
  test "$with_ffmpeg" = "autodetect" && problem_ffmpeg=" (libavformat not found by pkg-config)"
  with_ffmpeg="no"
else
  with_ffmpeg="yes"
fi 

# --- VNC 3.3.7

problem_vncplugin=" (--with-vnc option not or badly used)"
with_vncplugin="no"
HAVE_VNC=0
AC_ARG_WITH(vnc,
  [  --with-vnc=DIR          VNC 3.3.7 sources (compiled) are in DIR ],
  [if test x$with_vnc != xyes && test x$with_vnc != xno && test -d $with_vnc; then
     VNC_CPPFLAGS="-I$with_vnc/rfb"
     VNC_LDFLAGS="-L$with_vnc/rfb"
     HAVE_VNC=1
     with_vncplugin="yes"
     problem_vncplugin=""
   fi],
  HAVE_VNC=0)
test "$with_vnc" = "no" && problem_vncplugin=""
AC_SUBST(HAVE_VNC)
AC_SUBST(VNC_LDFLAGS)
AC_SUBST(VNC_CPPFLAGS)
AM_CONDITIONAL(HAVE_VNC,[test "x$HAVE_VNC" = "x1"])

# --- Qt

AC_ARG_WITH(qt,
  AS_HELP_STRING([--with-qt], [build Qt with support (default=autodetect)]),
  with_qt=$withval,
  with_qt="autodetect")

if test "$with_qt" = "autodetect" || test "$with_qt" = "yes"; then
  MY_PKG_CONFIG_CHECK([QtCore], HAVE_QT, QT)
  QT4MOC="$(pkg-config --variable=prefix QtCore)/bin/moc"
else
  HAVE_QT=0
fi
if test "$HAVE_QT" = "0" && test "$with_qt" = "yes"; then
  AC_MSG_ERROR("could not find qt")
fi
AC_SUBST(HAVE_QT)
AC_SUBST(QT_LDFLAGS)
AC_SUBST(QT_CPPFLAGS)
AC_SUBST(QT_LIBS)
AC_SUBST(QT4MOC)
AC_DEFINE_UNQUOTED(HAVE_QT,$HAVE_QT,[Qt support])
AM_CONDITIONAL(HAVE_QT,[test "x$HAVE_QT" = "x1"])
if test $HAVE_QT = 0; then
  test "$with_qt" = "autodetect" && problem_qt=" (QtCore not found by pkg-config)"
  with_qt="no"
else
  with_qt="yes"
fi

# --- gd

AC_ARG_WITH(gd,
  AS_HELP_STRING([--with-gd], [build with GD support (default=autodetect)]),
  with_gd=$withval,
  with_gd="autodetect")

HAVE_GD=0
if test "$with_gd" = "autodetect" || test "$with_gd" = "yes"; then
  AC_CHECK_PROG(GDLIB_CONFIG, gdlib-config, yes, no)
  if test "$GDLIB_CONFIG" = "yes"; then
    HAVE_GD=1
    GD_CPPFLAGS="`gdlib-config --cflags`"
    GD_LDFLAGS="`gdlib-config --ldflags`"
    GD_LIBS="`gdlib-config --libs` -lgd"
  fi
fi
if test "$HAVE_GD" = "0" && test "$with_gd" = "yes"; then
  AC_MSG_ERROR("could not find gd-config")
fi
AC_SUBST(HAVE_GD)
AC_SUBST(GD_CPPFLAGS)
AC_SUBST(GD_LDFLAGS)
AC_SUBST(GD_LIBS)
AC_DEFINE_UNQUOTED(HAVE_GD,$HAVE_GD,[GD support])
AM_CONDITIONAL(HAVE_GD,[test "x$HAVE_GD" = "x1"])
if test $HAVE_GD = 0; then
  test "$with_gd" = "autodetect" && problem_gd=" (gdlib-config not found)"
  with_gd="no"
else
  with_gd="yes"
fi

# --- OpenCV

AC_ARG_WITH(opencv,
  AS_HELP_STRING([--with-opencv], [build with OpenCV support (default=autodetect)]),
  with_opencv=$withval,
  with_opencv="autodetect")

if test "$with_opencv" = "autodetect" || test "$with_opencv" = "yes"; then
  MY_PKG_CONFIG_CHECK([opencv], HAVE_OPENCV, OPENCV)
  OPENCV_PREFIX=`pkg-config --variable=prefix opencv`
else
  HAVE_OPENCV=0
fi
if test "$HAVE_OPENCV" = "1" && test "$HAVE_FREETYPE2" = "0"; then
  AC_MSG_ERROR("opencv demos require freetype2 support")
fi
if test "$HAVE_OPENCV" = "0" && test "$with_opencv" = "yes"; then
  AC_MSG_ERROR("could not find opencv")
fi
AC_SUBST(HAVE_OPENCV)
AC_SUBST(OPENCV_LDFLAGS)
AC_SUBST(OPENCV_CPPFLAGS)
AC_SUBST(OPENCV_LIBS)
AC_SUBST(OPENCV_PREFIX)
AC_DEFINE_UNQUOTED(HAVE_OPENCV,$HAVE_OPENCV,[OpenCV support])
AM_CONDITIONAL(HAVE_OPENCV,[test "x$HAVE_OPENCV" = "x1"])
if test $HAVE_OPENCV = 0; then
  test "$with_opencv" = "autodetect" && problem_opencv=" (opencv not found by pkg-config)"
  with_opencv="no"
else
  with_opencv="yes"
fi

# --- Tests

AC_ARG_WITH(tests,
  AS_HELP_STRING([--with-tests], [build tests (default=yes)]),
  with_tests=$withval,
  with_tests="yes")

AM_CONDITIONAL(BUILD_TESTS,[test "x$with_tests" = "xyes"])

# --------------------------------------------------------

AC_CONFIG_FILES(
  Makefile
  nucleo/nucleo.H
  nucleo/Makefile
  nucleo/core/Makefile
  nucleo/core/select/Makefile
  nucleo/core/corefoundation/Makefile
  nucleo/core/carbon/Makefile
  nucleo/core/cocoa/Makefile
  nucleo/utils/Makefile
  nucleo/xml/Makefile
  nucleo/network/Makefile
  nucleo/network/udp/Makefile
  nucleo/network/tcp/Makefile
  nucleo/network/http/Makefile
  nucleo/network/dnssd/Makefile
  nucleo/network/xmpp/Makefile
  nucleo/image/Makefile
  nucleo/image/encoding/Makefile
  nucleo/image/source/Makefile
  nucleo/image/sink/Makefile
  nucleo/image/processing/Makefile
  nucleo/image/processing/basic/Makefile
  nucleo/image/processing/chromakeying/Makefile
  nucleo/image/processing/convolution/Makefile
  nucleo/image/processing/difference/Makefile
  nucleo/image/processing/gamma/Makefile
  nucleo/image/processing/noise/Makefile
  nucleo/gl/Makefile
  nucleo/gl/glIncludes.H
  nucleo/gl/window/Makefile
  nucleo/gl/texture/Makefile
  nucleo/gl/text/Makefile
  nucleo/gl/scenegraph/Makefile
  nucleo/gl/shading/Makefile
  nucleo/gl/window/keysym.H
  nucleo/helpers/Makefile
  nucleo/plugins/Makefile
  nucleo/plugins/vnc/Makefile
  nucleo/plugins/ffmpeg/Makefile
  nucleo/plugins/qt/Makefile
  nucleo/plugins/gd/Makefile
  tests/Makefile
  demos/Makefile
  demos/video/Makefile
  demos/misc/Makefile
  demos/vnc/Makefile
  demos/opencv/Makefile
  demos/opencv/faceFinder/Makefile
  demos/opencv/opticalFlow/Makefile
  apps/Makefile
  apps/videoServer/Makefile
  data/Makefile
  data/vera/Makefile
  docs/Makefile
  docs/MacOSX/Makefile
  docs/MacOSX/nTest
  docs/MacOSX/nBundle
  build-utils/Makefile
  build-utils/rpm/Makefile
  build-utils/rpm/nucleo.spec
  nucleo-config
  nucleo.pc
)

AC_OUTPUT

echo "
Núcleo configuration:

  Architecture: $host
  ------
  glWindow backend?     $with_opengl$problem_opengl
  JPEG image support?   $with_jpg$problem_jpg
  PNG image support?    $with_png$problem_png
  EXIF support?         $with_exif$problem_exif
  Freetype2 support?    $with_freetype2$problem_freetype2
  DNS-SD support?       $with_dns_sd$problem_dns_sd
  <eXpat/> support?     $with_expat$problem_expat
  GnuTLS support?       $with_gnutls$problem_gnutls
  ---
  FFmpeg plugin?        $with_ffmpeg$problem_ffmpeg
  VNC plugin?           $with_vncplugin$problem_vncplugin
  Qt plugin?            $with_qt$problem_qt
  GD plugin?            $with_gd$problem_gd
  ------
  Build OpenCV demos?   $with_opencv$problem_opencv
  Build tests?          $with_tests$problem_tests
"
