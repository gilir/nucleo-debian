/*
 *
 * nucleo/image/sink/nudpImageSink.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/sink/nudpImageSink.H>
#include <nucleo/image/encoding/Conversion.H>

#include <stdexcept>

#include <unistd.h>

namespace nucleo {

  nudpImageSink::nudpImageSink(const URI &uri) {
    _hostOrGroup = uri.host ; 
    _port = uri.port ;

    std::string query = uri.query ;

    std::string encoding ;
    _encoding = Image::JPEG ;    
    if (URI::getQueryArg(query, "encoding", &encoding))
	 _encoding = Image::getEncodingByName(encoding) ;
    
    _quality = 60 ;
    URI::getQueryArg(query, "quality", &_quality) ;

    _ttl = 0 ;
    URI::getQueryArg(query, "ttl", &_ttl) ;

    _udp = 0 ;
  }

  nudpImageSink::nudpImageSink(const char *hostOrGroup, int port,
						 Image::Encoding encoding, unsigned int quality,
						 unsigned int ttl) {
    _hostOrGroup = hostOrGroup ;
    _port = port ;
    _encoding = encoding ;
    _quality = quality ;
    _ttl = ttl ;
    _udp = 0 ;
  }

  nudpImageSink::~nudpImageSink(void) {
    stop() ;
  }

  bool
  nudpImageSink::start(void) {
    if (_udp) return false ;

    try {
	 _udp = new UdpSender(_hostOrGroup.c_str(), _port) ;
    } catch (std::runtime_error e) {
	 std::cerr << "nudpImageSink: " << e.what() << std::endl ;
    }
    if (!_udp) return false ;

    int sndbuf ;
    for (unsigned int i=30; i>0; --i) {
	 sndbuf = 1 << i ;
	 if (_udp->setBufferSize(sndbuf)) break ;
    }
    // std::cerr << "nudpImageSink: SO_SNDBUF=" << sndbuf << std::endl ;

    if (_ttl) _udp->setMulticastTTL(_ttl) ;

    frameCount = 0 ; sampler.start() ;
    return true ;
  }

  bool
  nudpImageSink::stop(void) {
    delete _udp ;
    _udp = 0 ;
    sampler.stop() ;
    return true ;
  }

  bool
  nudpImageSink::handle(Image *img) {
    if (!_udp) return false ;

    Image copy(*img) ;
    if (!convertImage(&copy, _encoding, _quality)) return false ;

    // std::cerr << "sending " << copy.getSize() << " bytes" << std::endl ;
    if (_udp->send(copy.getData(), copy.getSize())<=0) return false ;

    frameCount++ ; sampler.tick() ;
    return true ;
  }
    
}
