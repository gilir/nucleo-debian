/*
 *
 * nucleo/image/source/vssImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/source/vssImageSource.H>
#include <nucleo/utils/FileUtils.H>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace nucleo {

  // ------------------------------------------------------------------

  vssImageSource::vssImageSource(const URI &uri, Image::Encoding encoding) {
    target_encoding = encoding ;
    _filename = uri.opaque!="" ? uri.opaque : uri.path ;
    _deltat = 0 ; _syncFromData = true ;
    _keepReading = false ;
    _fd = -1 ;
    _tk = 0 ;

    double dArg ;
    if (URI::getQueryArg(uri.query, "framerate", &dArg) && dArg) {
	 _syncFromData = false ;
	 _deltat = (unsigned long)(1000.0 / dArg)*TimeKeeper::millisecond ;
    }
    URI::getQueryArg(uri.query, "keepreading", &_keepReading) ;
  }

  bool
  vssImageSource::start(void) {
    if (_fd!=-1) return false ;

    _msg.reset(true) ;
    _fd = open(_filename.c_str(), O_RDONLY) ;
    if (_fd==-1) {
	 std::cerr << "vssImageSource: no such file (" << _filename << ")" << std::endl ;
	 return false ;
    }
    _tk = TimeKeeper::create(_deltat, true) ;
    subscribeTo(_tk) ;

    frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;

    return true ;
  }

  void
  vssImageSource::react(Observable *obs) {
    if (_tk && obs!=_tk) return ;

    if (_msg.getState()==HttpMessage::COMPLETE) _msg.next() ;
   
    while (_msg.parseData()!=HttpMessage::COMPLETE) {
	 int nbb = _msg.feedFromStream(_fd) ;
	 if (nbb<1) {
	   if (_keepReading) return ;
	   if (_msg.completeData()!=HttpMessage::COMPLETE) {
		stop() ;
		notifyObservers() ;
		return ;
	   }
	   break ;
	 }
    }

    TimeStamp::inttype timestamp = TimeStamp::undef ;
    unsigned int width=0, height=0 ;
    std::string mimetype ;
    Image::Encoding encoding = Image::JPEG ;
    _msg.getHeader("nucleo-timestamp", &timestamp) ;
    _msg.getHeader("nucleo-image-width", &width) ;
    _msg.getHeader("nucleo-image-height", &height) ;
    if (_msg.getHeader("content-type", &mimetype))
	 encoding = Image::getEncodingByMimeType(mimetype) ;

    std::string const &body = _msg.body() ;
    lastImage.setEncoding(encoding==Image::OPAQUE ? Image::JPEG : encoding) ;
    lastImage.setDims(width, height) ;
    lastImage.setTimeStamp(timestamp!=TimeStamp::undef ? timestamp : TimeStamp::createAsInt()) ;
    lastImage.setData((unsigned char *)body.c_str(), body.length(), Image::NONE) ;
    frameCount++ ; sampler.tick() ;
    if (!_pendingNotifications) notifyObservers() ;
    
    if (_syncFromData) {
	 double framerate = 0.0 ;
	 if (!_msg.getHeader("nucleo-framerate", &framerate))
	   _msg.getHeader("videoSpace-framerate", &framerate) ;
	 if (framerate!=0.0)
	   _tk->arm((unsigned long)(1000.0/framerate)*TimeKeeper::millisecond, true) ;
    }
  }

  bool
  vssImageSource::getNextImage(Image *img, TimeStamp::inttype reftime) {
    if (!_tk || !frameCount || reftime>=lastImage.getTimeStamp()) return false ;
    previousImageTime = lastImage.getTimeStamp() ;
    bool ok = convertImage(&lastImage, target_encoding) ;
    if (ok) img->linkDataFrom(lastImage) ;
    return ok ;
  }

  bool
  vssImageSource::stop(void) {
    if (!_tk) return false ;

    sampler.stop() ;
    close(_fd) ;
    _fd = -1 ;
    unsubscribeFrom(_tk) ;
    delete _tk ;
    _tk = 0 ;

    return true ;
  }

  // ------------------------------------------------------------------

}
