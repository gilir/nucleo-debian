/*
 *
 * nucleo/image/source/novImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/source/novImageSource.H>
#include <nucleo/image/sink/novImageSink.H>

#include <nucleo/utils/FileUtils.H>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace nucleo {

  // ------------------------------------------------------------------

  novImageSource::novImageSource(const URI &uri, Image::Encoding encoding) : ImageSource() {
    target_encoding = encoding ;

    filename = uri.opaque!="" ? uri.opaque : uri.path ;
    
    default_framerate = 0.0 ; 
    URI::getQueryArg(uri.query, "framerate", &default_framerate) ;
    if (default_framerate<0) default_framerate = 0.0 ;

    default_speed = 1.0 ; 
    URI::getQueryArg(uri.query, "speed", &default_speed) ;
    if (default_speed<=0) default_speed = 1.0 ;

    keepReading = false ; 
    URI::getQueryArg(uri.query, "keepreading", &keepReading) ;

    previousImageRefTime = TimeStamp::undef ;
    tKeeper = 0 ;

    status = STOPPED ;
  }

  // ------------------------------------------------------------------

  bool novImageSource::preroll(void) {
    // std::cerr << "novImageSource::preroll: " << images.size() << " images" << std::endl ;
    int prfd = open(filename.c_str(), O_RDONLY) ;
    if (prfd==-1) {
	 std::cerr << "novImageSource::preroll: no such file (" << filename << ")" << std::endl ;
	 return false ;
    }

    off_t offset = 0 ;
    if (!images.empty())
	 offset = lseek(prfd, (*images.rbegin()).second, SEEK_SET) ;
    while (offset!=-1) {
	 novImageSink::ImageDescription desc ;
	 ssize_t nbbytes = read(prfd, &desc, sizeof(desc)) ;
	 if (nbbytes==-1 || nbbytes!=sizeof(desc)) break ;
	 desc.swapifle() ;
	 images[desc.timestamp] = offset ;
#if 0
	 std::cerr << "novImageSource::preroll: adding"
			 << desc.img_width << "x" << desc.img_height
			 << " " << Image::getEncodingName(desc.img_encoding) 
			 << " at " << offset
			 << " (" << TimeStamp::createAsStringFrom(desc.timestamp) << ")"
			 << std::endl ;
#endif
	 offset = lseek(prfd, desc.img_size+desc.xtra_size, SEEK_CUR) ;
    }

    close(prfd) ;

    return true ;
  }

  bool
  novImageSource::readImageAtOffset(off_t offset, Image *image) {
    if (status==STOPPED) return false ;
    off_t o = lseek(fd, offset, SEEK_SET) ;
    if (o!=offset) {
	 std::cerr << "novImageSource::readImageAtOffset: lseek returned " << o << " instead of " << offset << std::endl ;
	 return false ;
    }

    novImageSink::ImageDescription desc ;
    ssize_t nbbytes = read(fd, &desc, sizeof(desc)) ;
    if (nbbytes==-1 || nbbytes!=sizeof(desc)) {
	 std::cerr << "novImageSource::readImageAtOffset: read " << nbbytes << " bytes instead of " << sizeof(desc) << std::endl ;
	 return false ;
    }
    desc.swapifle() ;

    image->prepareFor(desc.img_width, desc.img_height, desc.img_encoding) ;
    if (image->getSize()!=desc.img_size)
	 image->setData(Image::AllocMem(desc.img_size), desc.img_size, Image::FREEMEM) ;
    nbbytes = read(fd, image->getData(), desc.img_size) ; 
    if (nbbytes==-1 || nbbytes!=(ssize_t)desc.img_size) {
	 std::cerr << "novImageSource::readImageAtOffset: read " << nbbytes << " bytes instead of " << desc.img_size << std::endl ;
	 return false ;   
    }
    image->setTimeStamp(desc.timestamp) ;

    return true ;
  }

  novImageSource::ImageIndex::iterator
  novImageSource::readImageAtTime(TimeStamp::inttype t, Image *image) {
    if (status==STOPPED) return images.end() ;
    ImageIndex::iterator i = images.upper_bound(t) ;
    if (i==images.end()) {
	 std::cerr << "novImageSource::readImageAtTime: can't find image past " << TimeStamp::createAsStringFrom(t) << std::endl ;
	 return images.end() ;
    }
    if (!readImageAtOffset((*i).second, image)) return images.end() ;
    return i ;
  }

  // ------------------------------------------------------------------

  bool
  novImageSource::start(void) {
    if (status!=STOPPED) return false ;

    fd = open(filename.c_str(), O_RDONLY) ;
    if (fd==-1) {
	 std::cerr << "novImageSource::start: no such file (" << filename << ")" << std::endl ;
	 return false ;
    }

    tKeeper = TimeKeeper::create() ;
    subscribeTo(tKeeper) ;
    setSpeed(default_speed) ;
    if (default_framerate<=0) 
	 tKeeper->arm(30*TimeKeeper::millisecond) ;
    else
	 setRate(default_framerate) ;

    frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;
    previousImageRefTime = TimeStamp::undef ;

    status = STARTED ;
    return true ;
  }

  bool
  novImageSource::stop(void) {
    if (status==STOPPED) return false ;

    sampler.stop() ; previousImageTime = TimeStamp::undef ; lastImage.clear() ;
    images.clear() ; previousImageRefTime = TimeStamp::undef ;

    if (tKeeper) { unsubscribeFrom(tKeeper) ; delete tKeeper ; tKeeper=0 ; }

    close(fd) ;
    status = STOPPED ;
    return true ;
  }

  // ------------------------------------------------------------------

  void
  novImageSource::react(Observable *obs) {
    if (tKeeper && obs==tKeeper) {
	 std::cerr << "novImageSource::react:" << std::endl ;
	 preroll() ;
	 notifyObservers() ;
    }
  }

  bool
  novImageSource::getNextImage(Image *img, TimeStamp::inttype reftime) {
    if (status==STOPPED) {
	 std::cerr << "novImageSource::getNextImage: bad conditions" << std::endl
			 << "   " << TimeStamp::createAsStringFrom(reftime) << " " << TimeStamp::createAsStringFrom(lastImage.getTimeStamp())
			 << std::endl ;
	 
	 return false ;
    }

    TimeStamp::inttype t ;
    if (previousImageRefTime==TimeStamp::undef) {
	 t = TimeStamp::undef ;
    } else {
	 TimeStamp::inttype delta = (TimeStamp::createAsInt() - previousImageRefTime)*speed ;
	 t = previousImageTime + delta ;
    }
    ImageIndex::iterator i = readImageAtTime(t, &lastImage) ;
    if (i==images.end()) {
	 std::cerr << "novImageSource::getNextImage: readImageAtTime failed" << std::endl ;
	 return false ;
    }

    img->linkDataFrom(lastImage) ;
    previousImageTime = lastImage.getTimeStamp() ;
    previousImageRefTime = TimeStamp::createAsInt() ;
    frameCount++ ;

    std::cerr << "novImageSource::getNextImage: framerate = " << framerate << std::endl ;
    if (framerate<=0) {
	 i++ ;
	 if (i==images.end())
	   tKeeper->arm(30*TimeKeeper::millisecond) ;
	 else
	   tKeeper->arm(((*i).first-previousImageTime)*TimeKeeper::millisecond/speed) ;
    }

    std::cerr << "novImageSource::getNextImage: youpee" << std::endl ;    
    return true ;
  }

  // ------------------------------------------------------------------

  bool
  novImageSource::setRate(double r) {
    framerate = r ;
    if (tKeeper) {
	 if (framerate>0)
	   tKeeper->arm((1.0/framerate)*TimeKeeper::second, true) ;
	 else
	   tKeeper->arm(30*TimeKeeper::millisecond) ;
    }
    return true ;
  }

  bool
  novImageSource::setSpeed(double s) {
    speed = s ;
    return true ;
  }

  // ------------------------------------------------------------------

  TimeStamp::inttype
  novImageSource::getStartTime(void) {
    if (images.empty()) preroll() ;
    if (images.empty()) return TimeStamp::undef ;
    return (*images.begin()).first ;
  }

  TimeStamp::inttype
  novImageSource::getDuration(void) {
    if (images.empty()) preroll() ;
    std::vector<ImageIndex>::size_type s = images.size() ;
    if (s<2) return TimeStamp::undef ;
    return (*images.rbegin()).first - (*images.begin()).first ;
  }

  bool
  novImageSource::setTime(TimeStamp::inttype t) {
    if (status==STOPPED) return false ;

    if (images.empty()) preroll() ;
    ImageIndex::iterator i = images.lower_bound(t) ;
    if (i==images.end()) return false ;

    // std::cerr << "setTime: " << TimeStamp::createAsStringFrom(t) << " --> " << TimeStamp::createAsStringFrom((*i).first) << std::endl ;
    return true ;    
  }

  // ------------------------------------------------------------------

}
