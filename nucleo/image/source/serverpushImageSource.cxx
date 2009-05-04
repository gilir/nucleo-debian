/*
 *
 * nucleo/image/source/serverpushImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/source/serverpushImageSource.H>

#include <nucleo/utils/FileUtils.H>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdexcept>
#include <sstream>

namespace nucleo {

  // ------------------------------------------------------------------

  serverpushImageSource::serverpushImageSource(const URI &uri, Image::Encoding encoding) : ImageSource() {
    target_encoding = encoding ; 

    _hostname = uri.host ;
    if (_hostname=="") _hostname="localhost" ;
    _port = uri.port ;
    if (!_port) _port = 80 ;

    std::string resource = uri.path ;
    if (resource=="") resource = "/" ;
    std::string query = uri.query ;
    if (query!="") resource = resource + "?" + query ;
    std::stringstream request ;
    request << "GET " << resource << " HTTP/1.1" << oneCRLF
		  << "User-Agent: Mozilla/1.1 nucleo/" << PACKAGE_VERSION << oneCRLF
		  << "Accept: image/jpeg" << oneCRLF
#if 0
		  << "Connection: keep-alive" << oneCRLF
		  << "Connection: close" << oneCRLF
#endif
		  << "Host: " << _hostname << oneCRLF
		  << oneCRLF ;
    _request = request.str() ;

    _state = CLOSED ;
    _connection = 0 ;
  }

  bool
  serverpushImageSource::start(void) {
    if (_state!=CLOSED) return false ;

    _msg.reset(true) ;
    try {
	 _connection = new TcpConnection(_hostname, _port) ;
    } catch (std::runtime_error e) {
	 std::cerr << "serverpushImageSource: " << e.what() << std::endl ;
	 return false ;
    } catch (...) {
	 return false ;
    }

    subscribeTo(_connection) ;
	
    try {
	 _connection->send((const char*) _request.c_str(), _request.length()) ;
    } catch (std::runtime_error e) {
	 std::cerr << "serverpushImageSource: " << e.what() << std::endl ;
	 delete _connection ;
	 return false;
    } catch (...) {
	 return false ;
    }
	
    _state = OPEN ;
    frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;
    return true ;
  }

  void
  serverpushImageSource::react(Observable *obs) {
    if (obs!=_connection) return ;

    if (_msg.getState()==HttpMessage::COMPLETE) _msg.next() ;
    if (_msg.feedFromStream(_connection->getFd())<1) _state = BUFFERED ;
    HttpMessage::State s = _msg.parseData() ;

    if (s==HttpMessage::NEED_BODY && _state==BUFFERED) s=_msg.completeData() ;
    if (s!=HttpMessage::COMPLETE) {
	 if (_state==BUFFERED) { stop() ; notifyObservers() ; }
	 return ;
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
  }

  bool
  serverpushImageSource::getNextImage(Image *img, TimeStamp::inttype reftime) {
    if (_state==CLOSED || !frameCount || reftime>=lastImage.getTimeStamp()) return false ;
    previousImageTime = lastImage.getTimeStamp() ;
    bool ok = convertImage(&lastImage, target_encoding) ;
    if (ok) img->linkDataFrom(lastImage) ;
    return ok ;
  }

  bool
  serverpushImageSource::stop(void) {
    if (_state!=CLOSED) {
	 sampler.stop() ;
	 unsubscribeFrom(_connection) ;
	 delete _connection ;
	 _msg.reset(true) ;
	 _state = CLOSED ;
	 return true ;
    }
    return false ;
  }

  // ------------------------------------------------------------------

}
