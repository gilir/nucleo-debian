#ifndef n_qtMovImageSource_H
#define n_qtMovImageSource_H

/*
 *
 * nucleo/image/source/qtMovImageSource.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/source/ImageSource.H>
#include <nucleo/core/URI.H>
#include <nucleo/core/TimeKeeper.H>

#include <QuickTime/QuickTime.h>

namespace nucleo {

  class qtMovImageSource : public ImageSource {

  private:

    MovieController player ;
    Movie movie ;
    MovieDrawingCompleteUPP callback ;
    Image::Encoding qtEncoding ;

    TimeKeeper *timer ;

  protected:

    void react(Observable*) ;

    friend pascal OSErr DrawCompleteProc(Movie theMovie, long ptr) ;

  public:

    qtMovImageSource(const URI &uri, Image::Encoding e) ;
    ~qtMovImageSource() ;

    ImageSource::state getState(void)  { return (timer?STARTED:STOPPED) ; }

    bool start(void) ;
    bool getNextImage(Image *img, TimeStamp::inttype reftime) ;
    bool stop(void) ;

    TimeStamp::inttype getDuration(void) ;

  } ;

}

#endif
