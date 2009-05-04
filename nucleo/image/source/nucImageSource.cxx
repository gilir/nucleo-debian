/*
 *
 * nucleo/image/source/nucImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/source/nucImageSource.H>
#include <nucleo/utils/FileUtils.H>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace nucleo {

  // ------------------------------------------------------------------

  nucImageSource::nucImageSource(const URI &uri, Image::Encoding encoding) : ImageSource() {
    target_encoding = encoding ;
    current = next = 0 ;
    filename = uri.opaque!="" ? uri.opaque : uri.path ;
    framerate = 0.0 ;
    speed = 1.0 ;
    fdKeeper = 0 ;
    frKeeper = psKeeper = 0 ;
    keepReading = false ;
    state = STOPPED ;

    URI::getQueryArg(uri.query, "framerate", &framerate) ;
    URI::getQueryArg(uri.query, "speed", &speed) ;
    URI::getQueryArg(uri.query, "keepreading", &keepReading) ;

    if (framerate<0) framerate = 0.0 ;
    if (speed<=0) speed = 1.0 ;
  }

  // ------------------------------------------------------------------

  void
  nucImageSource::flushImages(void) {
    bool delete_next = (current!=next) ;
    delete current ;
    current = 0 ;
    if (delete_next) delete next ;
    next = 0 ;
  }

  void
  nucImageSource::watchFd(bool watchIt) {
    // std::cerr << "nucImageSource::watchFd: " << watchIt << ", state: " << state << ", fdKeeper: " << fdKeeper << ", frKeeper: " << frKeeper << ", psKeeper: " << psKeeper << std::endl ;
    if (watchIt && !fdKeeper) {
	 fdKeeper = FileKeeper::create(fd, FileKeeper::R) ;
	 subscribeTo(fdKeeper) ;
    }
    if (!watchIt && fdKeeper) {
	 unsubscribeFrom(fdKeeper) ;
	 delete fdKeeper ;
	 fdKeeper = 0 ;
    }
  }
  
  Image *
  nucImageSource::readImage(void) {
    msg.next() ;

    while (msg.parseData()!=HttpMessage::COMPLETE) {
	 int nbb = msg.feedFromStream(fd) ;
	 if (nbb<1) {
	   if (!keepReading && msg.completeData()!=HttpMessage::COMPLETE) {
		stop() ;
		notifyObservers() ;
	   }
	   return 0 ;
	 }
    }

    TimeStamp::inttype timestamp = TimeStamp::undef ;
    unsigned int width=0, height=0 ;
    std::string mimetype ;
    Image::Encoding encoding = Image::JPEG ;
    msg.getHeader("nucleo-timestamp", &timestamp) ;
    msg.getHeader("nucleo-image-width", &width) ;
    msg.getHeader("nucleo-image-height", &height) ;
    if (msg.getHeader("content-type", &mimetype))
	 encoding = Image::getEncodingByMimeType(mimetype) ;

    Image *image = new Image ;
    std::string const &body = msg.body() ;
    image->setEncoding(encoding==Image::OPAQUE ? Image::JPEG : encoding) ;
    image->setDims(width, height) ;
    image->setTimeStamp(timestamp!=TimeStamp::undef ? timestamp : TimeStamp::createAsInt()) ;
    image->setData((unsigned char *)body.c_str(), body.length(), Image::NONE) ;
    if (!_pendingNotifications) notifyObservers() ;

    return image ;
  }

  // ------------------------------------------------------------------

  bool
  nucImageSource::start(void) {
    if (state!=STOPPED) return false ;

    flushImages() ;
    msg.reset(true) ;

    fd = open(filename.c_str(), O_RDONLY) ;
    if (fd==-1) {
	 std::cerr << "nucImageSource: no such file (" << filename << ")" << std::endl ;
	 return false ;
    }

    watchFd(true) ;
    setRate(framerate) ;
    setSpeed(speed) ;
    psKeeper = TimeKeeper::create() ;
    subscribeTo(psKeeper) ;

    state = STARTED ;
    frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;

    return true ;
  }

  bool
  nucImageSource::stop(void) {
    if (state==STOPPED) return false ;
    sampler.stop() ;
    flushImages() ;
    msg.reset(true) ;
    if (fdKeeper) { unsubscribeFrom(fdKeeper) ; delete fdKeeper ; fdKeeper=0 ; }
    if (frKeeper) { unsubscribeFrom(frKeeper) ; delete frKeeper ; frKeeper=0 ; }
    if (psKeeper) { unsubscribeFrom(psKeeper) ; delete psKeeper ; psKeeper=0 ; }
    close(fd) ;

    state = STOPPED ;
    return true ;
  }

  // ------------------------------------------------------------------

  void
  nucImageSource::react(Observable *obs) {
    // std::cerr << "nucImageSource::react: " << obs << " (" << fdKeeper << ", " << frKeeper << ", " << psKeeper << ")" << std::endl ;

    if (!current) current = readImage() ;
    if (current && !next) next = readImage() ;
    // std::cerr << "react, line " << __LINE__ << ": current=" << current << ", next=" << next << std::endl ;    

    if (current && next) {
	 TimeStamp::inttype t0 = current->getTimeStamp() ;
	 double epsilon=20.0, delta ;
	 unsigned int nbSkipped=0 ;
	 do {
	   delta = (next->getTimeStamp()-t0)/speed ;    
	   if (delta<epsilon) {
		nbSkipped++ ;
		// std::cerr << "skip: " << delta << std::endl ;
		delete next ;
		next = 0 ; // because readImage might want to delete it
		next = readImage() ;
	   }
	 } while (delta<epsilon && next) ;
	 // if (nbSkipped) std::cerr << "Skipped " << nbSkipped << " images" << std::endl ;
	 if (next) {
	   next->acquireData() ;
	   watchFd(false) ;
	   psKeeper->arm((unsigned long)delta) ;
	 }
    }

    if (current) {
	 if (convertImage(current, &lastImage, target_encoding)) {
	   if (lastImage.dataIsLinked()) lastImage.acquireData() ;
	   frameCount++ ; sampler.tick() ;
	   notifyObservers() ;
	 }
    }

    if (!next && state!=STOPPED) watchFd(true) ;

    delete current ;
    current = next ;
    next = 0 ;
  }

  bool
  nucImageSource::getNextImage(Image *img, TimeStamp::inttype reftime) {
    if (state==STOPPED || !frameCount || reftime>=lastImage.getTimeStamp()) return false ;
    previousImageTime = lastImage.getTimeStamp() ;
    img->linkDataFrom(lastImage) ;
    return true ;
  }

  // ------------------------------------------------------------------

  void
  nucImageSource::getStartStopTimes(TimeStamp::inttype *startTime, TimeStamp::inttype *stopTime) {
    if (!startTime && !stopTime) return ;
    if (startTime) *startTime = TimeStamp::undef ;
    if (stopTime) *stopTime = TimeStamp::undef ;

    int fd = open(filename.c_str(), O_RDONLY) ;
    unsigned int nbImages = 0 ;
    HttpMessage msg ;
    for (bool loop=true; loop;) {
	 int nbb = msg.feedFromStream(fd) ;
	 if (nbb<1) { msg.completeData() ; loop = false ; }
	 if (msg.parseData()==HttpMessage::COMPLETE) {
	   TimeStamp::inttype timestamp = TimeStamp::undef ;
	   msg.getHeader("nucleo-timestamp", &timestamp) ;
	   if (!nbImages) {
		if (startTime) *startTime = timestamp ; 
		off_t cur = lseek(fd, 0, SEEK_CUR) ;
		cur = lseek(fd, (off_t)(-2.33*cur), SEEK_END) ;
	   } else {
		if (stopTime) *stopTime = timestamp ;
	   }
	   msg.next(true) ;
	   nbImages++ ;
	 }
    }
    close(fd) ;
  }

  TimeStamp::inttype
  nucImageSource::getStartTime(void) {
    TimeStamp::inttype startTime ;
    getStartStopTimes(&startTime, 0) ;
    return startTime ;
  }

  TimeStamp::inttype
  nucImageSource::getDuration(void) {
    TimeStamp::inttype startTime, stopTime ;
    getStartStopTimes(&startTime, &stopTime) ;
    return (stopTime-startTime) ;
  }

  bool
  nucImageSource::setTime(TimeStamp::inttype t) {
    if (state==STOPPED) return false ;

    TimeStamp::inttype startTime, stopTime ;
    getStartStopTimes(&startTime, &stopTime) ;
    if (t<startTime || t>stopTime) return false ;

    // std::cerr << TimeStamp::createAsStringFromInt(startTime) << " - " << TimeStamp::createAsStringFromInt(stopTime) << std::endl ;
    
    off_t old = lseek(fd, 0, SEEK_CUR) ;

    uint64_t fileSize = getFileSize(filename.c_str()) ;
    off_t offset = (off_t)(fileSize*(t-startTime)/(stopTime-startTime)) ;
    if (offset != lseek(fd, offset, SEEK_SET)) {
	 lseek(fd, old, SEEK_SET) ;
	 return false ;
    }

    flushImages() ;
    msg.next(true) ;
    return true ;    
  }

  // ------------------------------------------------------------------

  bool
  nucImageSource::setRate(double r) {
    if (frKeeper) {
	 unsubscribeFrom(frKeeper) ;
	 delete frKeeper ;
	 frKeeper = 0 ;
    }
    framerate = r ;
    if (framerate>0) {
	 frKeeper = TimeKeeper::create((1.0/framerate)*TimeKeeper::second, true) ;
	 subscribeTo(frKeeper) ;
    }
    return true ;
  }

  bool
  nucImageSource::setSpeed(double s) {
    speed = s ;
    return true ;
  }

  // ------------------------------------------------------------------

}
