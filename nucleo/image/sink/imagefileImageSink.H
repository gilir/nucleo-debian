#ifndef n_imagefileImageSink_H
#define n_imagefileImageSink_H

/*
 *
 * nucleo/image/sink/imagefileImageSink.H --
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

  class imagefileImageSink : public ImageSink {

  private:

    std::string _filename ;

    Image::Encoding _target_encoding ;
    unsigned int _quality ;

    int _fd ;
    bool _active ;

  public:

    imagefileImageSink(const URI &uri, Image::Encoding target_encoding) {
	 _filename = uri.opaque!="" ? uri.opaque : uri.path ;

	 if (!URI::getQueryArg(uri.query, "quality", &_quality)) _quality = 100 ;

	 _target_encoding = target_encoding ;

	 _active = false ;
    }

    ImageSink::state getState(void) {
	 return (_active?STARTED:STOPPED) ;
    }

    bool start(void) {
	 if (_active) return false ;

	 _fd = createFile(_filename.c_str()) ;
	 if (_fd==-1) return false ;

	 _active = true ;
	 frameCount = 0 ; sampler.start() ;
	 return true ;
    }

    bool handle(Image *img) {
	 if (!_active) return false ;

	 Image tmp(*img) ;
	 convertImage(&tmp, _target_encoding, _quality) ;
	 write(_fd, tmp.getData(), tmp.getSize()) ;
	 frameCount++ ; sampler.tick() ;
	 stop() ;
	 return true ;
    }

    bool stop(void) {
	 if (!_active) return false ;

	 close(_fd) ;
	 _active = false ;
	 sampler.stop() ;
	 return true ;
    }

  } ;

}

#endif
