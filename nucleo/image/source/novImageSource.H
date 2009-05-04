#ifndef n_novImageSource_H
#define n_novImageSource_H

/*
 *
 * nucleo/image/source/novImageSource.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/TimeKeeper.H>
#include <nucleo/core/FileKeeper.H>
#include <nucleo/core/TimeStamp.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/core/URI.H>
#include <nucleo/image/encoding/Conversion.H>

#include <map>

#include <unistd.h>

namespace nucleo {

  // ------------------------------------------------------------------

  class novImageSource : public ImageSource {

  private:

    ImageSource::state status ;

    std::string filename ;
    double default_framerate, framerate ;
    double default_speed, speed ;
    bool keepReading ;

    int fd ;

    typedef std::map<TimeStamp::inttype, off_t> ImageIndex ;
    ImageIndex images ;

    TimeStamp::inttype previousImageRefTime ;
    TimeKeeper *tKeeper ;

  protected:

    bool readImageAtOffset(off_t offset, Image *image) ;
    ImageIndex::iterator readImageAtTime(TimeStamp::inttype t, Image *image) ;
    bool preroll(void) ;

    void react(Observable*) ;

  public:

    novImageSource(const URI &uri, Image::Encoding e) ;
    ~novImageSource(void) { stop() ; }

    ImageSource::state getState(void) { return status ; }

    bool start(void) ;
    using ImageSource::getNextImage ;
    bool getNextImage(Image *img, TimeStamp::inttype reftime) ;
    bool stop(void) ;

    TimeStamp::inttype getStartTime(void) ;
    TimeStamp::inttype getDuration(void) ;
    bool setTime(TimeStamp::inttype t) ;
    bool setRate(double r) ;
    bool setSpeed(double s) ;

  } ;

  // ------------------------------------------------------------------

}

#endif
