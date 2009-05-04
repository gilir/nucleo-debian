#ifndef n_nudpcImageSource_H
#define n_nudpcImageSource_H

/*
 *
 * nucleo/image/source/nudpcImageSource.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/source/ImageSource.H>
#include <nucleo/core/URI.H>
#include <nucleo/network/http/HttpMessage.H>
#include <nucleo/network/tcp/TcpConnection.H>
#include <nucleo/network/udp/UdpReceiver.H>

#include <nucleo/image/encoding/Conversion.H>

namespace nucleo {

  // ------------------------------------------------------------------

  class nudpcImageSource : public ImageSource {

  private:

    std::string _hostname ;
    int _port ;
    TcpConnection *_tcp ;
    std::string _request ;

    UdpReceiver *_udp ;

  protected:

    void react(Observable*) ;

  public:

    nudpcImageSource(const URI &uri, Image::Encoding encoding) ;
    ~nudpcImageSource() { stop() ; }

    ImageSource::state getState(void) { return (_tcp ? STARTED : STOPPED) ; }

    bool start(void) ;
    bool getNextImage(Image *img, TimeStamp::inttype reftime) ;
    bool stop(void) ;

  } ;

  // ------------------------------------------------------------------

}

#endif
