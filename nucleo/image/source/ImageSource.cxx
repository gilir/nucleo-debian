/*
 *
 * nucleo/image/source/ImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/core/ReactiveEngine.H>

#include <nucleo/image/source/ImageSource.H>

#ifdef __APPLE__
#include <nucleo/image/source/qtMovImageSource.H>
// #include <nucleo/image/source/oldQtSgImageSource.H>
#include <nucleo/image/source/qtSgImageSource.H>
#endif

#ifdef __linux__
#include <nucleo/image/source/v4lImageSource.H>
#endif

#include <nucleo/image/source/nudpImageSource.H>
#include <nucleo/image/source/nudppImageSource.H>
#include <nucleo/image/source/nudpcImageSource.H>

#include <nucleo/image/source/serverpushImageSource.H>

#include <nucleo/image/source/imagefileImageSource.H>
#include <nucleo/image/source/vssImageSource.H>
#include <nucleo/image/source/nucImageSource.H>
#include <nucleo/image/source/novImageSource.H>

#include <nucleo/image/source/noiseImageSource.H>

#include <nucleo/core/PluginManager.H>

#include <string>
#include <stdexcept>

namespace nucleo {

  // ------------------------------------------------------------------

  bool
  ImageSource::waitForImage(Image *img) {
    while (getState()!=ImageSource::STOPPED) {
	 if (getNextImage(img)) return true ;
	 ReactiveEngine::step(100) ;
    }
    return false ;
  }

  // ------------------------------------------------------------------

  bool
  ImageSource::getImage(const char *uri, Image *img, Image::Encoding enc) {
    ImageSource *source = ImageSource::create(uri, enc) ;
    source->start() ;
    bool gotTheImage = source->waitForImage(img) ;
    if (gotTheImage && img->dataIsLinked()) img->acquireData() ;
    delete source ;
    return gotTheImage ;
  }

  // ------------------------------------------------------------------

  ImageSource *
  ImageSource::create(const char *u, Image::Encoding e) {
    if (!u) throw std::runtime_error("Can't create an ImageSource from an empty URI...") ;

    URI uri(u) ;

    std::string scheme = uri.scheme ;
    if (scheme=="") scheme = "file" ;

    // -------------------------------------------------------

    ImageSourceFactory factory = 0 ;

#ifdef __APPLE__
    // if (scheme == "oldvideoin") return new oldQtSgImageSource(uri,e) ;
    if (scheme == "videoin") return new qtSgImageSource(uri,e) ;
#endif
#ifdef __linux__
    if (scheme == "videoin") return new v4lImageSource(uri,e) ;
#endif

    if (scheme=="noise") return new noiseImageSource(uri,e) ;

    if (scheme=="file") {
	 std::string filename = uri.opaque!="" ? uri.opaque : uri.path ;

	 if (fileIsDir(filename.c_str()))
	   throw std::runtime_error("createImageSource: file is a directory") ;

	 const char *extension = getExtension(filename.c_str()) ;
	 if (!extension)
	   throw std::runtime_error("createImageSource: unknown file type '"+filename+"'") ;
	 cistring iext = extension ;
	 if (iext == ".nov") return new novImageSource(uri,e) ;
	 if (iext == ".nuc") return new nucImageSource(uri,e) ;
	 if (iext == ".vss") return new vssImageSource(uri,e) ;
	 if (iext == ".jpg" || iext == ".jpeg") return new imagefileImageSource(uri, Image::JPEG, e) ;
	 if (iext == ".png") return new imagefileImageSource(uri, Image::PNG, e) ;
	 if (iext == ".pam") return new imagefileImageSource(uri, Image::PAM, e) ;
#ifdef __APPLE__
	 try {
	   return new qtMovImageSource(uri,e) ;
	 } catch (...) {
	   std::cerr << "Failed to create a qtMovImageSource from " << filename << std::endl ;
	 }
#endif
    }

    if (scheme == "nudp") return new nudpImageSource(uri,e) ;
    if (scheme == "nudpc") return new nudpcImageSource(uri,e) ;
    if (scheme == "nudpp") return new nudppImageSource(uri,e) ;
    if (scheme == "http") return new serverpushImageSource(uri,e) ;

    factory = (ImageSourceFactory) PluginManager::getSymbol("ImageSource::create",
												std::string("scheme=")+scheme) ;
    return (ImageSource*) ((*factory)(&uri, e)) ;
  }

  // ------------------------------------------------------------------

}
