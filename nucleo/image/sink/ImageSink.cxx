/*
 *
 * nucleo/image/ImageSink.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/sink/ImageSink.H>

#ifdef __APPLE__
#include <nucleo/image/sink/qtMovImageSink.H>
#endif

#include <nucleo/image/sink/imagefileImageSink.H>
#include <nucleo/image/sink/serverpushImageSink.H>
#include <nucleo/image/sink/novImageSink.H>

#if HAVE_OPENGL
#include <nucleo/image/sink/glwindowImageSink.H>
#endif

#include <nucleo/image/sink/nudpImageSink.H>
#include <nucleo/image/sink/nudppImageSink.H>
#include <nucleo/image/sink/nserverImageSink.H>

#include <nucleo/image/sink/bufferedImageSink.H>
#include <nucleo/image/sink/blackholeImageSink.H>

#include <nucleo/core/PluginManager.H>

#include <stdexcept>

namespace nucleo {

  // ---------------------------------------------------------------------------

  ImageSink *
  ImageSink::create(const char *u) {
    if (!u) throw std::runtime_error("Can't create an ImageSink from an empty URI...") ;

    URI uri(u) ;
    std::string scheme = uri.scheme ;
    if (scheme=="") scheme = "file" ;

    // -------------------------------------------------------

    if (scheme=="file") {
	 std::string filename = uri.opaque!="" ? uri.opaque : uri.path ;
	 const char *extension = getExtension(filename.c_str()) ;
	 if (!extension) throw std::runtime_error("createImageSink: unknown file type '"+filename+"'") ;
	 cistring iext = extension ;
#if 1
	 if (iext == ".vss") return new serverpushImageSink(uri) ;
#else
	 if (iext == ".vss")
	   throw std::runtime_error(".vss video format is not supported anymore. Use .nuc instead.") ;
#endif
	 if (iext == ".nov") return new novImageSink(uri) ;
	 if (iext == ".nuc") return new serverpushImageSink(uri) ;
	 if (iext == ".jpg") return new imagefileImageSink(uri, Image::JPEG) ;
	 if (iext == ".jpeg") return new imagefileImageSink(uri, Image::JPEG) ;
	 if (iext == ".png") return new imagefileImageSink(uri, Image::PNG) ;
	 if (iext == ".pam") return new imagefileImageSink(uri, Image::PAM) ;
#ifdef __APPLE__
	 if (iext == ".mov") return new qtMovImageSink(uri) ;
#endif
    } else if (scheme=="nudp")
	 return new nudpImageSink(uri) ;
    else if ((scheme=="nudpp") || (scheme=="nudp+"))
	 return new nudppImageSink(uri) ;
#if HAVE_OPENGL
    else if (scheme=="glwindow") return new glwindowImageSink(uri) ;
#endif
    else if (scheme=="nserver") return new nserverImageSink(uri) ;
    else if (scheme=="buffered") return new bufferedImageSink(uri) ;
    else if (scheme=="blackhole" || scheme=="devnull") return new blackholeImageSink(uri) ;

    std::string tag = std::string("scheme=")+scheme ;
    ImageSinkFactory factory = (ImageSinkFactory) PluginManager::getSymbol("ImageSink::create",tag) ;
    return (ImageSink *) (*factory) (&uri) ;
  }

  // ---------------------------------------------------------------------------

}
