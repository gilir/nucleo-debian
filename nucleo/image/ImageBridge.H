#ifndef n_ImageBridge_H
#define n_ImageBridge_H

/*
 *
 * nucleo/image/ImageBridge.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/sink/ImageSink.H>

namespace nucleo {

  // ------------------------------------------------------------------

  class ImageBridge : public ReactiveObject {

  protected:

    ImageSource *source ;
    Image image ;
    ImageSink *sink ;
    bool ownSource, ownSink ;

    virtual void react(Observable *obs) {
	 if (source->getNextImage(&image)) {
	   sink->handle(&image) ;
	 } else if (source->getState()==ImageSource::STOPPED || sink->getState()==ImageSink::STOPPED)
	   notifyObservers() ;
    }

  private:

    void init(ImageSource *src, ImageSink *dst) {
	 source = src ;
	 sink = dst ;
	 subscribeTo(sink) ;
	 sink->start() ;
	 subscribeTo(source) ;
	 source->start() ;
    }

  public:

    ImageBridge(std::string src, std::string dst) {
	 ownSource = ownSink = true ;
	 init(ImageSource::create(src), ImageSink::create(dst)) ;
    }

    ImageBridge(ImageSource *src, ImageSink *dst, bool dSrc=true, bool dDst=true) {
	 ownSource = dSrc ;
	 ownSink = dDst ;
	 init(src, dst) ;
    }

    bool isActive(void) {
	 return (source->getState()!=ImageSource::STOPPED && sink->getState()!=ImageSink::STOPPED) ;
    }

    ~ImageBridge(void) {
	 unsubscribeFrom(source) ;
	 if (ownSource) delete source ;
	 unsubscribeFrom(sink) ;
	 if (ownSink) delete sink ;
    }

  } ;

  // ------------------------------------------------------------------

}

#endif
