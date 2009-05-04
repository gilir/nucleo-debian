#ifndef n_serverpushImageSink_H
#define n_serverpushImageSink_H

/*
 *
 * nucleo/image/sink/serverpushImageSink.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/URI.H>
#include <nucleo/image/sink/ImageSink.H>
#include <nucleo/network/http/ServerPush.H>

namespace nucleo {

  class serverpushImageSink : public ImageSink {

  private:

    unsigned int _quality ;
    Image::Encoding _encoding ;

    int _fd ;
    ServerPush *_spush ;
    bool _close, _active ;

  public:

    serverpushImageSink(const URI &uri) ;
    serverpushImageSink(int fd,
				    Image::Encoding encoding,
				    unsigned int quality,
				    bool closeAfter=false) ;
    ~serverpushImageSink(void) ;

    ImageSink::state getState(void) {
	 return (_active?STARTED:STOPPED) ;
    }

    bool start(void) ;
    bool handle(Image *img) ;
    bool stop(void) ;
    
  } ;

}

#endif
