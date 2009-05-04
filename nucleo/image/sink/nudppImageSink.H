#ifndef n_nudppImageSink_H
#define n_nudppImageSink_H

/*
 *
 * nucleo/image/sink/nudppImageSink.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/sink/ImageSink.H>

#include <netinet/in.h>

namespace nucleo {

  class nudppImageSink : public ImageSink {
  
  private:

    int _socket ;
    struct sockaddr_in _peer ;

    Image::Encoding _encoding ;
    unsigned int _quality ;

    bool _active ;

  public:

    struct FragmentInfo {
	 uint16_t inum, fnum ;
	 uint32_t encoding ;
	 uint16_t width, height ;
	 uint32_t totalsize ;
    } ;

    static unsigned int MaxFragmentSize ;

    nudppImageSink(const URI &uri) ;
    ~nudppImageSink() ;

    ImageSink::state getState(void) {
	 return (_active?STARTED:STOPPED) ;
    }

    bool start(void) ;
    bool handle(Image *img) ;
    bool stop(void) ;
  
  } ;

}

#endif
