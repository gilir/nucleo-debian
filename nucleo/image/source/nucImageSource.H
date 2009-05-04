#ifndef n_nucImageSource_H
#define n_nucImageSource_H

/*
 *
 * nucleo/image/source/nucImageSource.H --
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

  class nucImageSource : public ImageSource {

  private:

    ImageSource::state state ;

    std::string filename ;
    double framerate, speed ;
    bool keepReading ;

    int fd ;
    HttpMessage msg ;
    FileKeeper *fdKeeper ;
    TimeKeeper *frKeeper ; // frame rate
    TimeKeeper *psKeeper ; // playback speed

    Image *current, *next ;

    std::string getFilename(void) ;
    void getStartStopTimes(TimeStamp::inttype *startTime, TimeStamp::inttype *stopTime) ;

    void flushImages(void) ;
    void watchFd(bool b) ;
    Image *readImage(void) ;

  protected:

    void react(Observable*) ;

  public:

    nucImageSource(const URI &uri, Image::Encoding e) ;
    ~nucImageSource(void) { stop() ; }

    ImageSource::state getState(void) { return state ; }

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
