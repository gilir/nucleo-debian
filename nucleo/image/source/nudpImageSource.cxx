/*
 *
 * nucleo/image/source/nudpImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/source/nudpImageSource.H>
#include <nucleo/network/NetworkUtils.H>

#include <sstream>
#include <stdexcept>

namespace nucleo {

  // ------------------------------------------------------------------

  void
  nudpImageSource::init(std::string hostOrGroup, int port,
				    Image::Encoding s_encoding,
				    Image::Encoding t_encoding) {
    _hostOrGroup = hostOrGroup ; 
    _port = port ;
    source_encoding = s_encoding ;
    target_encoding = t_encoding==Image::PREFERRED ? s_encoding : t_encoding ;

    _udp = 0 ;
  }

  nudpImageSource::nudpImageSource(std::string hostOrGroup, int port,
							Image::Encoding source_encoding,
							Image::Encoding target_encoding) {
    init(hostOrGroup, port, source_encoding, target_encoding) ;
  }

  nudpImageSource::nudpImageSource(const URI &uri, Image::Encoding target_encoding) {
    Image::Encoding source_encoding = Image::JPEG ;    
    std::string encoding ;
    if (URI::getQueryArg(uri.query, "encoding", &encoding))
	 source_encoding = Image::getEncodingByName(encoding) ;

    init(uri.host, uri.port, source_encoding, target_encoding) ;
  }

  bool
  nudpImageSource::start(void) {
    if (_udp) return false ;

    try {
	 if (_hostOrGroup=="" || _hostOrGroup=="localhost" || _hostOrGroup=="127.0.0.1") {
	   _hostOrGroup = getHostName() ;
	   _udp = new UdpReceiver(_port) ;
	 } else
	   _udp = new UdpReceiver(_hostOrGroup.c_str(), _port) ;
	 
	 int rcvbuf ;
	 for (unsigned int i=30; i>0; --i) {
	   rcvbuf = 1<<i ;
	   if (_udp->setBufferSize(rcvbuf)) break ;
	 }
	 // std::cerr << "nudpImageSource: SO_RCVBUF=" << rcvbuf << std::endl ;
	 subscribeTo(_udp) ;

	 frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;
	 return true ;
    } catch (std::runtime_error e) {
	 std::cerr << "nudpImageSource: " << e.what() << std::endl ;
    } catch(...) {
    }

    delete _udp ;
    return false ;
  }

  void
  nudpImageSource::react(Observable *obs) {
    if (_udp && obs==_udp) {
	 unsigned char *_data = 0 ;
	 unsigned int _dataSize = 0 ;
	 if (_udp->receive(&_data, &_dataSize)) {
	   lastImage.setEncoding(source_encoding) ;
	   lastImage.setData(_data, _dataSize, Image::DELETE) ;
	   lastImage.setTimeStamp() ;
	   frameCount++ ; sampler.tick() ; 
	   if (!_pendingNotifications) notifyObservers() ;
	 }
    }
  }

  bool
  nudpImageSource::getNextImage(Image *img, TimeStamp::inttype reftime) {
    if (!frameCount || reftime>=lastImage.getTimeStamp()) return false ;
    previousImageTime = lastImage.getTimeStamp() ;
    bool ok = convertImage(&lastImage, target_encoding) ;
    if (ok) img->linkDataFrom(lastImage) ;
    return ok ;
  }

  bool
  nudpImageSource::stop(void) {
    if (!_udp) return false ;

    sampler.stop() ;

    unsubscribeFrom(_udp) ;
    delete _udp ;
    _udp = 0 ;

    return true ;
  }

  std::string
  nudpImageSource::getURI(void) {
    std::stringstream result ;
    int port = _udp ? _udp->getPortNumber() : _port ;
    result << "nudp://" << _hostOrGroup ;
    if (port) result << ":" << port ;
    return result.str() ;
  }

  // ------------------------------------------------------------------

}
