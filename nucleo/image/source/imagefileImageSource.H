#ifndef n_imagefileImageSource_H
#define n_imagefileImageSource_H

/*
 *
 * nucleo/image/source/imagefileImageSource.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/FileUtils.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/core/URI.H>
#include <nucleo/image/encoding/Conversion.H>

#include <stdexcept>

namespace nucleo {

  class imagefileImageSource : public ImageSource {

  private:

    std::string filename ;
    Image::Encoding source_encoding ;

  public:

    imagefileImageSource(const URI &uri,
					Image::Encoding s_encoding, Image::Encoding t_encoding) {
	 filename = uri.opaque!="" ? uri.opaque : uri.path ;
	 source_encoding = s_encoding ;
	 target_encoding = t_encoding ;
    }

    bool start(void) {
	 if (frameCount==1) return false ;

	 unsigned int size = getFileSize(filename.c_str()) ;
	 unsigned char *data = Image::AllocMem(size) ;
	 readFromFile(filename.c_str(), data, size) ;
	 lastImage.setEncoding(source_encoding) ;
	 lastImage.setData(data, size, Image::FREEMEM) ;
	 lastImage.setTimeStamp(getFileTime(filename.c_str())) ;
	 frameCount = 0 ; previousImageTime = TimeStamp::undef ;
	 if (convertImage(&lastImage, target_encoding)) {
	   frameCount++ ;
	   sampler.start() ; sampler.tick() ;
	   notifyObservers() ;
	   return true ;
	 }
	 return false ;
    }

    ImageSource::state getState(void) {
	 return frameCount?STARTED:STOPPED ;
    }

    bool getNextImage(Image *img, TimeStamp::inttype reftime) {
	 if (!frameCount || reftime>=lastImage.getTimeStamp()) return false ;
	 img->linkDataFrom(lastImage) ;
	 previousImageTime = lastImage.getTimeStamp() ;
	 if (_pendingNotifications<1) stop() ;
	 return true ;
    }

    bool stop(void) {
	 if (!frameCount) return false ;
	 sampler.stop() ;
	 frameCount = 0 ;
	 return true ;
    }

  } ;

  // ------------------------------------------------------------------

}

#endif
