/*
 *
 * nucleo/image/sink/nudppImageSink.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/sink/nudppImageSink.H>
#include <nucleo/utils/FileUtils.H>
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/network/NetworkUtils.H>

#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/ip.h>

#include <stdexcept>
#include <iostream>
#include <cstring>

#define DEBUG_LEVEL 0

namespace nucleo {

  // Max IP datagram size is 65535 bytes. IP header is 20 bytes and
  // UDP header is 8 bytes. That leaves us with at most 65507
  // bytes. However, the UDP/IP stack might support less than
  // that. 516 seems a minimum though...

  // sysctl (*udp*) and ifconfig (mtu) might be helpfull

  // static unsigned int base = 65535 ;
  // static unsigned int base = 32768 ;
  // static unsigned int base = 17000 ;
  // static unsigned int base = 16384 ;
  // static unsigned int base = 16000 ;
  // static unsigned int base = 9216 ;
  // static unsigned int base = 8192 ;
  static unsigned int base = 1500 ;
  // static unsigned int base = 1400 ;
  // static unsigned int base = 512 ;
  // static unsigned int base = 576 ;

  static unsigned int headers = 20+8 ;

  unsigned int nudppImageSink::MaxFragmentSize = base - headers - sizeof(nudppImageSink::FragmentInfo) ;

  nudppImageSink::nudppImageSink(const URI &uri) {
    std::string hostOrGroup = uri.host ; 
    int port = uri.port ;
    std::string query = uri.query ;

    _socket = socket(AF_INET, SOCK_DGRAM, 0) ;
    if (_socket<0) throw std::runtime_error("nudppImageSink: can't create socket") ;

    int sndbuf ;
    for (unsigned int i=30; i>0; --i) {
	 sndbuf = 1 << i ;
	 int res = setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, sizeof(sndbuf)) ;
	 if (res!=-1) break ;
    }

#if DEBUG_LEVEL>0
    std::cerr << "SO_SNDBUF=" << sndbuf << std::endl ;
    std::cerr << "MaxFragmentSize=" << MaxFragmentSize << std::endl ;
#endif

#if 0
  const int tosbits = IPTOS_LOWDELAY | IPTOS_THROUGHPUT ;
  if (setsockopt(_socket, IPPROTO_IP, IP_TOS, &tosbits, sizeof(tosbits))<0)
    std::cerr << "nudppImageSink: IP_TOS settings failed" << std::endl ;
#endif

    memset(&_peer, 0, sizeof(_peer)) ;
    _peer.sin_family = AF_INET ;
    _peer.sin_port = htons(port) ;
    _peer.sin_addr.s_addr = resolveAddress((char *)hostOrGroup.c_str()) ;

    std::string encoding ;
    _encoding = Image::JPEG ;    
    if (URI::getQueryArg(query, "encoding", &encoding))
	 _encoding = Image::getEncodingByName(encoding) ;
    
    _quality = 60 ;
    URI::getQueryArg(query, "quality", &_quality) ;

    unsigned int ttl ;
    if (URI::getQueryArg(query, "ttl", &ttl))
	 if (setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) == -1)
	   throw std::runtime_error("nudppImageSink: can't set TTL value") ;

    _active = false ;
  }

  bool
  nudppImageSink::start(void) {
    _active = true ;
    frameCount = 0 ; sampler.start() ;
    return true ;
  }

  bool
  nudppImageSink::stop(void) {
    sampler.stop() ;
    _active = false ;
    return true ;
  }

  bool
  nudppImageSink::handle(Image *tmp) {
    if (!_active) return false ;

    Image img ;
    convertImage(tmp, &img, _encoding) ;

    uint16_t frameNumber = (uint16_t)frameCount ;
    int fNum = 0 ;

    nudppImageSink::FragmentInfo info ;
    info.inum = htons(frameNumber) ;
    info.fnum = htons(fNum) ;
    info.encoding = htonl(img.getEncoding()) ;
    info.width = htons(img.getWidth()) ;
    info.height = htons(img.getHeight()) ;
    info.totalsize = htonl(img.getSize()) ;

    struct iovec iov[2] ;
    iov[0].iov_base = (char *)&info ;
    iov[0].iov_len = sizeof(info) ;

    struct msghdr msg ;
    memset(&msg, 0, sizeof (msg)) ;

    msg.msg_name = (caddr_t)&_peer ;
    msg.msg_namelen = sizeof(_peer) ;
    msg.msg_iov = iov ;
    msg.msg_iovlen = 2 ;

    // ---------------------------------------------

    unsigned int bytestosend = img.getSize() ;
    unsigned char *base = img.getData() ;

#if DEBUG_LEVEL>0
    std::cerr << frameNumber << " (" << bytestosend << "): " ;
#endif
    while (bytestosend>0) {
	 unsigned int len = bytestosend ;
	 if (len>MaxFragmentSize) len = MaxFragmentSize ;
#if DEBUG_LEVEL>0
	 std::cerr << fNum << "(" << len << ") " ;
#endif
	 iov[1].iov_base = (char *)base ;
	 iov[1].iov_len = len ;

	 ssize_t s = sendmsg(_socket, &msg, 0) ;
	 if (s==-1) {
	   perror("nudppImageSink") ;
	   return false ;
	 }

	 bytestosend -= len ;
	 base += len ;
	 info.fnum = htons(++fNum) ;
    }
#if DEBUG_LEVEL>0
    std::cerr << std::endl ;
#endif

    frameCount++ ; sampler.tick() ;  
    return true ;
  }

  nudppImageSink::~nudppImageSink() {
    shutdown(_socket,2) ;
    close(_socket) ;
  }

}
