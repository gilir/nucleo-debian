#ifndef n_AnimatedGifImageSink_H
#define n_AnimatedGifImageSink_H

/*
 *
 * nucleo/plugins/gd/AnimatedGifImageSink.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/sink/ImageSink.H>
#include <nucleo/core/URI.H>

#include <gd.h>

namespace nucleo {

  class AnimatedGifImageSink : public ImageSink {

  protected:

    ImageSink::state status ;
    std::string filename ;
    int loops ;
    double scale, speed ;
    TimeStamp::inttype lastTime ;
    gdImagePtr im ;
    FILE *out ;

  public:

    AnimatedGifImageSink(const URI &uri) ;
    ~AnimatedGifImageSink(void) ;
  
    state getState(void) ;

    bool start(void) ;
    bool handle(Image *img) ;
    bool stop(void) ;

  } ;

}

#endif
