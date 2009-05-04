#ifndef n_qtMovImageSink_H
#define n_qtMovImageSink_H

/*
 *
 * nucleo/image/sink/qtMovImageSink.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/sink/ImageSink.H>
#include <nucleo/core/URI.H>
#include <nucleo/core/TimeStamp.H>

#include <QuickTime/QuickTime.h>

#include <string>

namespace nucleo {

  class qtMovImageSink : public ImageSink {

  private:

    std::string _filename ;
    CodecType _codec ;
    CodecQ _quality ;

    Movie _movie ;
    Track _track ;
    Media _media ;
    DataHandler _dataHandler ;

    Image _image ;
    GWorldPtr _gworld ;
    PixMapHandle _pxmpHdl ;
    Handle _cmprDtHdl ;
    ImageDescriptionHandle _imgDscHdl ;
    int _framerate ;

    TimeStamp::inttype _lastTime ;

    enum {SEMI_OPENED, OPENED, CLOSED} _state ;

    bool createMovieFile(unsigned int width, unsigned int height) ;

  public:

    qtMovImageSink(const URI &u) ;
    ~qtMovImageSink() ;

    ImageSink::state getState(void) {
	 return (_state!=CLOSED?STARTED:STOPPED) ;
    }

    bool start(void) ;
    bool handle(Image *img) ;
    bool stop(void) ;
    
  } ;

}

#endif
