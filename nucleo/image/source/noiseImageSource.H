#ifndef n_noiseImageSource_H
#define n_noiseImageSource_H

/*
 *
 * nucleo/image/source/noiseImageSource.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/processing/noise/PerlinNoise.H>
#include <nucleo/core/TimeKeeper.H>

namespace nucleo {

  class noiseImageSource : public ImageSource {

  private:

    ImprovedPerlinNoise noisemaker, dx, dy ;
    unsigned int width, height ;
    unsigned int harmonics ;           // number of harmonics
    float scale ;                      // horizontal and vertical scale
    unsigned char min_val, max_val ;   // minimum and maximum values for the image

    double offset_x, offset_y ;
    unsigned long _deltat ;
    TimeKeeper *_tk ;

  protected:

    void react(Observable*) ;

  public:

    noiseImageSource(const URI &uri, Image::Encoding encoding) ;
    ~noiseImageSource(void) { stop() ; }

    ImageSource::state getState(void) { return (_tk?STARTED:STOPPED) ; }

    bool start(void) ;
    bool getNextImage(Image *img, TimeStamp::inttype reftime) ;
    bool stop(void) ;

  } ;

}

#endif
