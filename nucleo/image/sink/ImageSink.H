#ifndef vs_ImageSink_H
#define vs_ImageSink_H

/*
 *
 * nucleo/image/sink/ImageSink.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveObject.H>
#include <nucleo/utils/TimeUtils.H>
#include <nucleo/core/URI.H>
#include <nucleo/image/Image.H>

#include <string>

namespace nucleo {

  // ----------------------------------------------------------------------------

  class ImageSink : public ReactiveObject {

  public:

    typedef enum {STOPPED, STARTED} state ;

  protected:

    Chronometer sampler ;
    int64_t frameCount ;

  public:

    ImageSink(void) : frameCount(0) {}
    virtual ~ImageSink(void) {}

    virtual state getState(void) = 0 ;

    bool isActive(void) {
	 static bool deprecatedWarning = true ;
	 if (deprecatedWarning) {
	   std::cerr << "Warning: ImageSink::isActive is deprecated. Use ImageSink::getState instead" << std::endl ;
	   deprecatedWarning = false ;
	 }
	 return getState()!=STOPPED ;
    }

    virtual bool start(void) = 0 ;
    virtual bool handle(Image *img) = 0 ;
    virtual bool stop(void) = 0 ;

    int64_t getFrameCount(void) { return frameCount ; }

    // Mean (i.e. playback) frame rate
    void clearMeanRate(void) { sampler.start() ; }
    double getMeanRate(void) { return sampler.average() ; }

    static ImageSink *create(const char *uri) ;
    static ImageSink *create(std::string uri) {
	 return create(uri.c_str()) ;
    }

  } ;

  typedef ImageSink * (*ImageSinkFactory) (const URI *uri) ;

  // ----------------------------------------------------------------------------

}

#endif
