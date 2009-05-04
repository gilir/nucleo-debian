#ifndef n_nudpImageSink_H
#define n_nudpImageSink_H

/*
 *
 * nucleo/image/sink/nudpImageSink.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/sink/ImageSink.H>
#include <nucleo/network/udp/UdpSender.H>

namespace nucleo {

  // ------------------------------------------------------------------

  class nudpImageSink : public ImageSink {

  private:

    std::string _hostOrGroup ;
    int _port ;
    unsigned int _ttl ;
    unsigned int _quality ;
    Image::Encoding _encoding ;

    UdpSender *_udp ;

  public:

    nudpImageSink(const URI &uri) ;
    nudpImageSink(const char *hostOrGroup, int port,
			   Image::Encoding encoding, unsigned int quality,
			   unsigned int ttl) ;
    ~nudpImageSink(void) ;

    ImageSink::state getState(void) {
	 return (_udp!=0?STARTED:STOPPED) ;
    }

    bool start(void) ;
    // warning: only the *data* part of the image is to be transmitted
    // (not the width, height and encoding information)
    bool handle(Image *img) ;
    bool stop(void) ;
    
  } ;

  // ------------------------------------------------------------------

}

#endif
