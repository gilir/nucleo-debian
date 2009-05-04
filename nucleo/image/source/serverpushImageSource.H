#ifndef n_serverpushImageSource_H
#define n_serverpushImageSource_H

/*
 *
 * nucleo/image/source/serverpushImageSource.H --
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
#include <nucleo/image/encoding/Conversion.H>

namespace nucleo {

  // ------------------------------------------------------------------

  class serverpushImageSource : public ImageSource {

  private:

    std::string _hostname ;
    int _port ;
    TcpConnection *_connection ;
    std::string _request ;
    HttpMessage _msg ;

    enum {CLOSED, OPEN, READY, BUFFERED} _state ;

  protected:

    void react(Observable*) ;

  public:

    serverpushImageSource(const URI &uri, Image::Encoding e) ;
    ~serverpushImageSource(void) { stop() ; }

    ImageSource::state getState(void) { return (_state!=CLOSED ? STARTED : STOPPED) ; }

    bool start(void) ;
    bool getNextImage(Image *img, TimeStamp::inttype reftime) ;
    bool stop(void) ;

  } ;

  // ------------------------------------------------------------------

}

#endif
