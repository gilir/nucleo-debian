/*
 *
 * nucleo/image/source/nudpcImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/source/nudpcImageSource.H>

#include <nucleo/utils/FileUtils.H>

#include <netdb.h>
#include <cstdio>
#include <unistd.h>

#include <stdexcept>
#include <sstream>

namespace nucleo {

  // ------------------------------------------------------------------

  nudpcImageSource::nudpcImageSource(const URI &uri, Image::Encoding t_encoding) : ImageSource() {
    _hostname = uri.host ;
    if (_hostname=="") _hostname="localhost" ;
    _port = uri.port ;
    if (!_port) _port = 5555 ;

    target_encoding = (t_encoding==Image::PREFERRED ? Image::JPEG : t_encoding) ;

    _request = "/nudp" ;
    std::string tmp = uri.path ;
    if (tmp!="") _request.append(tmp) ; else _request.append("/video") ;
    _request.append("?") ;
    if (uri.query!="") {
	 _request.append(uri.query) ;
	 _request.append("&") ;
    }

    _tcp = 0 ;
    _udp = 0 ;
  }

  bool
  nudpcImageSource::start(void) {
    if (_tcp!=0) return false ;

    try {
	 _tcp = new TcpConnection(_hostname, _port) ;
    } catch (std::runtime_error e) {
	 std::cerr << "nudpcImageSource (start): " << e.what() << std::endl ;
	 return false ;
    } catch (...) {
	 return false ;
    }

    _udp = new UdpReceiver() ;

    // --- Get the local hostname and _udp port number -------------------

    char hostname[50] ;
    gethostname(hostname, 50) ;
    char udpOptions[255] ;

    hostent* local_hostent=gethostbyname(hostname) ;
    sprintf(udpOptions, "nudp=%d.%d.%d.%d%%3A%d", (unsigned char)(local_hostent->h_addr[0]), (unsigned char)(local_hostent->h_addr[1]), (unsigned char)(local_hostent->h_addr[2]), (unsigned char)(local_hostent->h_addr[3]), _udp->getPortNumber()) ;

    // ---------------------------------------------------------------------

    std::stringstream request ;
    request << "GET " << _request << udpOptions << " HTTP/1.1" << oneCRLF
		  << "User-Agent: nucleo/" << PACKAGE_VERSION << oneCRLF
		  << oneCRLF ;
    std::string theRequest = request.str() ;
    // std::cerr << theRequest << std::endl ;

    try {
	 _tcp->send((const char*) theRequest.c_str(), theRequest.length()) ; 
    } catch (std::runtime_error e) {
	 std::cerr << "nudpcImageSource (start, 2): " << e.what() << std::endl ;
	 stop() ;
	 return false ;
    } catch (...) {
	 stop() ;
	 return false ;
    }

    subscribeTo(_tcp) ;
    subscribeTo(_udp) ;

    frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;
    return true ;
  }

  void
  nudpcImageSource::react(Observable *obs) {
    if (_udp && obs==_udp) {
	 unsigned char *_data = 0 ;
	 unsigned int _dataSize = 0 ;
	 if (_udp->receive(&_data, &_dataSize)) {
	   lastImage.setEncoding(Image::JPEG) ;
	   lastImage.setData(_data, _dataSize, Image::DELETE) ;
	   lastImage.setTimeStamp() ;
	   frameCount++ ; sampler.tick() ; 
	   if (!_pendingNotifications) notifyObservers() ;
	 }
    }

    if (_tcp && obs==_tcp) {
	 HttpMessage msg ; 
	 msg.parseFromStream(_tcp->getFd()) ;
	 // could do something with msg...
	 stop() ;
	 notifyObservers() ;
    }
  }

  bool
  nudpcImageSource::getNextImage(Image *img, TimeStamp::inttype reftime) {
    if (!_tcp || !frameCount || reftime>=lastImage.getTimeStamp()) return false ;
    previousImageTime = lastImage.getTimeStamp() ;
    bool ok = convertImage(&lastImage, target_encoding) ;
    if (ok) img->linkDataFrom(lastImage) ;
    return ok ;
  }

  bool
  nudpcImageSource::stop(void) {
    if (!_tcp) return false ;

    sampler.stop() ;

    unsubscribeFrom(_udp) ;
    delete _udp ;
    _udp = 0 ;

    unsubscribeFrom(_tcp) ;
    delete _tcp ;
    _tcp = 0 ;

    return true ;
  }

  // ------------------------------------------------------------------

}
