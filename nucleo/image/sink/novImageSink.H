#ifndef n_novImageSink_H
#define n_novImageSink_H

/*
 *
 * nucleo/image/sink/novImageSink.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/sink/ImageSink.H>

namespace nucleo {

  class novImageSink : public ImageSink {

  private:

    std::string filename ;
    int fd ;
    Image::Encoding encoding ;
    unsigned int quality ;
    ImageSink::state status ;

  public:

    struct __attribute__ ((__packed__)) ImageDescription {
	 TimeStamp::inttype timestamp ; 
	 uint32_t img_size ;
	 Image::Encoding img_encoding ;
	 uint32_t img_width ;
	 uint32_t img_height ;
	 uint32_t xtra_size ;
	 // Total size should be sizeof(ImageDescription)+img_size+xtra_size

	 void swapifle(void) ;
    } ;

    novImageSink(const URI &uri) ;
    ~novImageSink(void) { stop() ; }

    ImageSink::state getState(void) { return status ; }

    bool start(void) ;
    bool handle(Image *img) { return handle(img,0,0) ; }
    bool handle(Image *img, void *xtra, uint32_t xtra_size) ;
    bool stop(void) ;
    
  } ;

}

#endif
