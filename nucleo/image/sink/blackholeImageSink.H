#ifndef n_blackholeImageSink_H
#define n_blackholeImageSink_H

/*
 *
 * nucleo/image/sink/blackholeImageSink.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/sink/ImageSink.H>
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/utils/FileUtils.H>

#include <unistd.h>
#include <stdexcept>

namespace nucleo {

  class blackholeImageSink : public ImageSink {

  protected:

    ImageSink::state status ;

  public:

    blackholeImageSink(const URI &uri) {
	 status = STOPPED ;
    }

    ImageSink::state getState(void) {
	 return status ;
    }

    bool start(void) {
	 if (status==STARTED) return false ;
	 status = STARTED ;
	 frameCount = 0 ;
	 sampler.start() ;
	 return true ;
    }

    bool handle(Image *img) {
	 sampler.tick() ;
	 return true ;
    }

    bool stop(void) {
	 if (status==STOPPED) return false ;
	 sampler.stop() ;
	 status = STOPPED ;
	 return true ;
    }

  } ;

}

#endif
