#ifndef n_bufferedImageSink_H
#define n_bufferedImageSink_H

/*
 *
 * nucleo/image/sink/bufferedImageSink.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/sink/ImageSink.H>

#include <queue>

namespace nucleo {

  class bufferedImageSink : public ImageSink {

  protected:

    ImageSink *sink ;
    bool buffering ;
    std::queue<Image*> images ;

    unsigned int max_size ;
    TimeStamp::inttype max_time ;
    bool blast ;

  public:

    bufferedImageSink(const URI &uri) ;
    ~bufferedImageSink(void) ;

    ImageSink::state getState(void) {
	 return (buffering?STARTED:STOPPED) ;
    }

    bool start(void) ;
    bool handle(Image *img) ;
    bool stop(void) ;

    void clear(void) ; // delete all buffered images
    bool flush(void) ; // send all buffered images to the underlying sink
    
  } ;

}

#endif
