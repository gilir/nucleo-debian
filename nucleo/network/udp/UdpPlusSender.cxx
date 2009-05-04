/*
 *
 * nucleo/network/udp/UdpPlusSender.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/udp/UdpPlusSender.H>
#include <nucleo/network/NetworkUtils.H>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <arpa/inet.h>

#include <string>
#include <stdexcept>
#include <iostream>
#include <cstring>

namespace nucleo {

#define DEBUG_LEVEL 0

  // ---------------------------------------------------------------------------------

  UdpPlusSender::UdpPlusSender(const char *hostOrGroup, int port) {
    _socket = socket(AF_INET, SOCK_DGRAM, 0) ;
    if (_socket<0) throw std::runtime_error("UdpPlusSender: can't create socket") ;

    int sndbuf ;
    for (unsigned int i=30; i>0; --i) {
	 sndbuf = 1 << i ;
	 int res = setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, sizeof(sndbuf)) ;
	 if (res!=-1) break ;
    }

#if DEBUG_LEVEL>0
    std::cerr << "UdpPlusSender: SO_SNDBUF=" << sndbuf << " FragmentSize=" << UdpPlus::FragmentSize << std::endl ;
#endif

    memset(&_peer, 0, sizeof(_peer)) ;
    _peer.sin_addr.s_addr = resolveAddress(hostOrGroup) ;
    _peer.sin_family = AF_INET ;
    _peer.sin_port = htons(port) ;
      
    _unum = 0 ;
  }

  // ---------------------------------------------------------------------------------

  void
  UdpPlusSender::setTTL(const unsigned char ttl) { 
    // 1=subnet, 31=site, 64=national, 127=worldwide
    if (setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) == -1)
	 throw std::runtime_error("UdpPlusSender: can't set TTL value") ;
  }

  bool
  UdpPlusSender::send(const void *content, unsigned int contentLength) {
    UdpPlus::Header header ;
    header.unum = htons(_unum) ;
    header.totalsize = htonl(contentLength) ;
    header.fnum = 0 ;

    struct iovec iov[2] ;
    iov[0].iov_base = (char *)&header ;
    iov[0].iov_len = sizeof(header) ;
    // ...

    struct msghdr msg ;
    memset(&msg, 0, sizeof (msg)) ;
    msg.msg_name = (caddr_t)&_peer ;
    msg.msg_namelen = sizeof(_peer) ;
    msg.msg_iov = iov ;
    msg.msg_iovlen = 2 ;

    // ---------------------------------------------

    unsigned int bytestosend = contentLength ;
    char *base = (char *)content ;

#if DEBUG_LEVEL>0
    std::cerr << header.unum << " (" << bytestosend << "): " ;
#endif
    while (bytestosend>0) {
	 unsigned int len = bytestosend ;
	 if (len>UdpPlus::FragmentSize) len = UdpPlus::FragmentSize ;
#if DEBUG_LEVEL>0
	 std::cerr << header.fnum << "(" << len << ") " ;
#endif
	 iov[1].iov_base = base ;
	 iov[1].iov_len = len ;

	 ssize_t s = sendmsg(_socket, &msg, 0) ;
	 if (s==-1) {
	   std::cerr << "UdpPlusSender::send(" << content << "," << contentLength << "): "
			   << strerror(errno) << std::endl ;
	   return false ;
	 }

	 bytestosend -= len ;
	 base += len ;
	 header.fnum++ ;
    }
#if DEBUG_LEVEL>0
    std::cerr << std::endl ;
#endif

    return true ;
  }

  // ---------------------------------------------------------------------------------

  UdpPlusSender::~UdpPlusSender() {
    shutdown(_socket,2) ;
    close(_socket) ;
  }

  // ---------------------------------------------------------------------------------

}
