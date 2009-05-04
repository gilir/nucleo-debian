/*
 *
 * nucleo/plugins/gd/AnimatedGifImageSink.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/plugins/gd/AnimatedGifImageSink.H>
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/image/processing/basic/Resize.H>

#include <stdexcept>

// ------------------------------------------------------------------------------------

extern "C" {
  void *
  AnimatedGifImageSink_factory(const nucleo::URI *uri) {
    return new nucleo::AnimatedGifImageSink(*uri) ;
  }
}

// ------------------------------------------------------------------------------------

namespace nucleo {

  AnimatedGifImageSink::AnimatedGifImageSink(const URI &uri) {
    filename = uri.opaque!="" ? uri.opaque : uri.path ;
    loops = -1 ; URI::getQueryArg(uri.query, "loops", &loops) ;
    scale = 1.0 ; URI::getQueryArg(uri.query, "scale", &scale) ;
    speed = 1.0 ; URI::getQueryArg(uri.query, "speed", &speed) ;
  }

  AnimatedGifImageSink::~AnimatedGifImageSink(void) {
    stop() ;
  }
  
  ImageSink::state
  AnimatedGifImageSink::getState(void) {
    return status ;
  }

  bool
  AnimatedGifImageSink::start(void) {
    if (status==STARTED) return false ;
    status = STARTED ;
    lastTime = TimeStamp::undef ;
    frameCount = 0 ; 
    sampler.start() ;
    return true ;
  }

  bool
  AnimatedGifImageSink::handle(Image *img) {
    if (status==STOPPED) return false ;

    Image local(*img) ;
    resizeImage(&local, scale*local.getWidth(), scale*local.getHeight()) ;
    if (local.getEncoding()!=Image::RGB && local.getEncoding()!=Image::ARGB)
	 if (!convertImage(&local,Image::ARGB)) return false ;
    unsigned int width = local.getWidth() ;
    unsigned int height = local.getHeight() ;

    if (!frameCount) {
	 im = gdImageCreateTrueColor(local.getWidth(), local.getHeight()) ;
	 if (filename=="stdout") 
	   out = stdout ;
	 else 
	   out = fopen(filename.c_str(), "wb") ;
	 gdImageGifAnimBegin(im, out, 0/*GlobalCM*/, loops) ;
    }

    gdImagePtr tmp = gdImageCreateTrueColor(width, height) ;
    unsigned char *ptr = local.getData() ;
    if (local.getEncoding()==Image::ARGB) {
	 for (unsigned int y=0; y<height; ++y)
	   for (unsigned int x=0; x<width; ++x) {
		ptr++ ; // Skip the alpha channel
		unsigned char r = *ptr++ ;
		unsigned char g = *ptr++ ;
		unsigned char b = *ptr++ ;
		tmp->tpixels[y][x] = gdTrueColor(r, g, b) ;
	   }
    } else {
	 for (unsigned int y=0; y<height; ++y)
	   for (unsigned int x=0; x<width; ++x) {
		unsigned char r = *ptr++ ;
		unsigned char g = *ptr++ ;
		unsigned char b = *ptr++ ;
		tmp->tpixels[y][x] = gdTrueColor(r, g, b);
	   }
    }

    TimeStamp::inttype thisTime = local.getTimeStamp() ;
    int duration = lastTime==TimeStamp::undef ? 1 : (thisTime-lastTime)/10 ; // 1/100s units
    duration = (int)(duration*(1.0/speed)) ;
    gdImageGifAnimAdd(tmp, out, 1/*LocalCM*/, 0,0, duration, gdDisposalNone, 0) ;

    gdImageDestroy(tmp) ;
    lastTime = thisTime ;

    frameCount++ ; sampler.tick() ;
    return true ;
  }

  bool
  AnimatedGifImageSink::stop(void) {
    if (status==STOPPED) return false ;
    sampler.stop() ;
    status = STOPPED ;
    gdImageGifAnimEnd(out) ;
    fclose(out) ;
    out = 0 ;
    gdImageDestroy(im) ;
    im = 0 ;
    return true ;
  }

}
