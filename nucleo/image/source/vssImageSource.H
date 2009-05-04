#ifndef n_vssImageSource_H
#define n_vssImageSource_H

/*
 *
 * nucleo/image/source/vssImageSource.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/TimeKeeper.H>
#include <nucleo/core/TimeStamp.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/core/URI.H>
#include <nucleo/network/http/HttpMessage.H>
#include <nucleo/image/encoding/Conversion.H>

namespace nucleo {

  // ------------------------------------------------------------------

  class vssImageSource : public ImageSource {

  private:

    std::string _filename ;
    unsigned long _deltat ;
    bool _syncFromData ;
    bool _keepReading ;

    int _fd ;
    HttpMessage _msg ;
    TimeKeeper *_tk ;

  protected:

    void react(Observable*) ;

  public:

    vssImageSource(const URI &uri, Image::Encoding e) ;
    ~vssImageSource(void) { stop() ; }

    ImageSource::state getState(void) { return (_tk?STARTED:STOPPED) ; }

    bool start(void) ;
    bool getNextImage(Image *img, TimeStamp::inttype reftime) ;
    bool stop(void) ;
    
  } ;

  // ------------------------------------------------------------------

}

#endif
