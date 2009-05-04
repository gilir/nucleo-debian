#ifndef n_nucleo_H
#define n_nucleo_H

/*
 *
 * nucleo/nucleo.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <string>

namespace nucleo {

#define NUCLEO_SUPPORTS_JPEG 1
#define NUCLEO_SUPPORTS_PNG 1
#define NUCLEO_SUPPORTS_EXIF 1

#define NUCLEO_SUPPORTS_OPENGL 1
#define NUCLEO_SUPPORTS_AGL 0
#define NUCLEO_SUPPORTS_GLX 1
#define NUCLEO_SUPPORTS_XINPUT 1
#define NUCLEO_SUPPORTS_FREETYPE2 1

#define NUCLEO_SUPPORTS_DNS_SD 1

#define NUCLEO_SUPPORTS_EXPAT 1
#define NUCLEO_SUPPORTS_GNUTLS 1

#define NUCLEO_SUPPORTS_VNC 0
#define NUCLEO_SUPPORTS_FFMPEG 1
#define NUCLEO_SUPPORTS_QT 1
#define NUCLEO_SUPPORTS_OPENCV 0

  std::string getNucleoVersion(void) ;
  std::string getNucleoPluginsDirectory(void) ;
  std::string getNucleoResourcesDirectory(void) ;

}

#endif
