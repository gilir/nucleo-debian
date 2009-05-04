#ifndef n_nudppImageSource_H
#define n_nudppImageSource_H

/*
 *
 * nucleo/image/source/nudppImageSource.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/sink/nudppImageSink.H>
#include <nucleo/core/FileKeeper.H>

namespace nucleo {

  class nudppImageSource : public ImageSource {

  private:

    std::string _hostOrGroup ;
    int _port, _realport ;
    int _socket ;
    FileKeeper *_skeeper ;
    Image::Encoding source_encoding ;

    nudppImageSink::FragmentInfo _info ;
    uint16_t _inum, _fnum ;
    unsigned int _bytesRead ;
    bool _newImage ;

    enum {CLOSED, OUT_OF_SYNC, SYNC} _state ;

    void _receiveFragment(void) ;

  protected:

    void react(Observable *) ;

  public:

    nudppImageSource(const URI &uri, Image::Encoding encoding) ;
    ~nudppImageSource() { stop() ; }

    ImageSource::state getState(void) { return (_state!=CLOSED?STARTED:STOPPED) ; }

    bool start(void) ;
    bool getNextImage(Image *img, TimeStamp::inttype reftime) ;
    bool stop(void) ;

    int getPortNumber(void) { return _realport ; }

  } ;

}

#endif
