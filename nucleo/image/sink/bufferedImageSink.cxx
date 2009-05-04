/*
 *
 * nucleo/image/sink/bufferedImageSink.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/sink/bufferedImageSink.H>

#define DEBUG_LEVEL 1

namespace nucleo {

  bufferedImageSink::bufferedImageSink(const URI &uri) {
#if DEBUG_LEVEL>0
    uri.debug(std::cerr) ; std::cerr << std::endl ;
#endif
    if (!URI::getQueryArg(uri.query, "maxs", &max_size)) max_size = 0 ;
    if (!URI::getQueryArg(uri.query, "maxt", &max_time)) max_time = 0 ;
    if (!URI::getQueryArg(uri.query, "blast", &blast)) blast = false ;
    sink = ImageSink::create(URI::decode(uri.opaque)) ;
    buffering = false ;
  }

  bool
  bufferedImageSink::start(void) {
    if (buffering) return false ;
    buffering = true ;
    frameCount = 0 ; sampler.start() ;
    return true ;
  }

  bool
  bufferedImageSink::handle(Image *img) {
    Image *dup = new Image ;
    dup->copyDataFrom(*img) ;
    images.push(dup) ;
    frameCount++ ; sampler.tick() ;
    if (max_size && images.size()>max_size) {
#if DEBUG_LEVEL>0
	 std::cerr << "bufferedImageSink: dropping an image" << std::endl ;
#endif
	 delete images.front() ;
	 images.pop() ;
    }
    
    if (max_time) {
	 TimeStamp::inttype t = img->getTimeStamp() ;
	 for (Image *i = images.front(); !images.empty(); i=images.front()) {
	   TimeStamp::inttype ti = i->getTimeStamp() ;
	   if (t==TimeStamp::undef || ti==TimeStamp::undef || ti>t || t-ti<max_time) break ;
#if DEBUG_LEVEL>0
	   std::cerr << "bufferedImageSink: dropping an image (t=" << ti << ")" << std::endl ;
#endif
	   delete i ;
	   images.pop() ;
	 }
    }
    return true ;
  }

  bool
  bufferedImageSink::stop(void) {
    if (!buffering) return false ;
    flush() ;
    buffering = false ;
    sink->stop() ;
    sampler.stop() ;
    return true ;
  }

  void
  bufferedImageSink::clear(void) {
#if DEBUG_LEVEL>0
    std::cerr << "bufferedImageSink: clearing " << images.size() << " images" << std::endl ;
#endif
    while (!images.empty()) {
	 Image *i = images.front() ;
	 delete i ;
	 images.pop() ;
    }
  }

  bool 
  bufferedImageSink::flush(void) {
    if (!buffering) return false ;

    if (sink->getState()==ImageSink::STOPPED) sink->start() ;

#if DEBUG_LEVEL>0
    std::cerr << "bufferedImageSink: flushing " << images.size() << " images" ;
    if (blast) std::cerr << " (blast!)" ;
    std::cerr << std::endl ;
#endif

    TimeStamp::inttype t_prev = TimeStamp::undef ;
    while (!images.empty()) {
	 Image *img = images.front() ;
	 images.pop() ;
	 if (!blast) {
	   TimeStamp::inttype t = img->getTimeStamp() ;
	   if (t_prev!=0 && t!=0) usleep((t-t_prev)*1000) ;
	   t_prev = t ;
	 }
	 bool ok = sink->handle(img) ;
	 delete img ;
	 if (!ok) {
#if DEBUG_LEVEL>0
	   std::cerr << "bufferedImageSink: sink refused the image" << std::endl ;
#endif
	   buffering = false ;
	   sink->stop() ;
	   clear() ;
	   return false ;
	 }
    }

#if DEBUG_LEVEL>0
    std::cerr << "bufferedImageSink: flushed" << std::endl ;
#endif

    return true ;
  }

  bufferedImageSink::~bufferedImageSink(void) {
    if (!stop()) clear() ;
    delete sink ;
  }

}
