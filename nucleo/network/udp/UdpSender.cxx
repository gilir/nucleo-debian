/*
 *
 * nucleo/network/udp/UdpSender.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/network/udp/UdpSender.H>
#include <nucleo/network/NetworkUtils.H>

#include <unistd.h>
#include <errno.h>

#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <cstring>

namespace nucleo {

  // -------------------------------------------------------------------------

  void
  UdpSender::init(const char *address, const int port) {
    _fd = socket(AF_INET, SOCK_DGRAM, 0) ;
    if (_fd<0) throw std::runtime_error("UdpSender: can't create socket") ;

    int sndbuf ;
    for (unsigned int i=30; i>0; --i) {
	 sndbuf = 1 << i ;
	 int res = setsockopt(_fd, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, sizeof(sndbuf)) ;
	 if (res!=-1) break ;
    }
    // std::cerr << "sndbuf=" << sndbuf << std::endl ;

    memset(&_peer, 0, sizeof(_peer)) ;
    _peer.sin_addr.s_addr = resolveAddress((char *)address) ;
    _peer.sin_family = AF_INET ;
    _peer.sin_port = htons(port) ;
  }

  bool
  UdpSender::setBufferSize(int size) {
    int res = setsockopt(_fd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size)) ;
    return (res!=-1) ;
  }

  void
  UdpSender::setMulticastTTL(unsigned int ttl) { 
    if (ttl>255) ttl = 255 ;
    unsigned char value = (unsigned char)ttl ;

    if (setsockopt(_fd, IPPROTO_IP, IP_MULTICAST_TTL, &value, 1) == -1)
	 throw std::runtime_error("UdpSender: can't set Multicast TTL value") ;
  }

  int
  UdpSender::send(const void *content, unsigned int contentLength) { 
    int result = sendto(_fd, content, contentLength, 0, (sockaddr *)&_peer, sizeof(_peer)) ;
    if (result!=(int)contentLength) {
	 std::cerr << "UdpSender::send(" << content << "," << contentLength << "): "
			 << strerror(errno) << std::endl ;
	 return 0 ;
    }
    return result ;
  }

  UdpSender::~UdpSender(void) {
    shutdown(_fd,2) ;
    ::close(_fd) ;
  }

  // -------------------------------------------------------------------------

}
