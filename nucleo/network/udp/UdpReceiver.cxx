/*
 *
 * nucleo/network/udp/UdpReceiver.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/network/udp/UdpReceiver.H>
#include <nucleo/network/NetworkUtils.H>
#include <nucleo/utils/FileUtils.H>

#include <arpa/inet.h>
#include <unistd.h>

#include <stdexcept>
#include <iostream>
#include <cstring>

namespace nucleo {

  // -------------------------------------------------------------------------

  void
  UdpReceiver::_open(const int port, const char *mcastGroup) {
    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socket<0) throw std::runtime_error("UdpReceiver: can't create socket") ;

    struct sockaddr_in name ;  
    memset(&name, 0, sizeof(name)) ;
    name.sin_family = AF_INET ;
    name.sin_addr.s_addr = htonl(INADDR_ANY) ;
    name.sin_port = htons(port) ;

    if (mcastGroup) {
	 int one=1 ;
	 // Allow more than one client on this host (multicast...)
#ifdef SO_REUSEPORT
	 setsockopt(_socket, SOL_SOCKET, SO_REUSEPORT, (char *)&one, sizeof(int)) ;
#endif
	 setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(int)) ;

	 // Request that the kernel join a multicast group
	 struct ip_mreq mreq ;
	 mreq.imr_multiaddr.s_addr = inet_addr(mcastGroup) ;
	 mreq.imr_interface.s_addr = htonl(INADDR_ANY) ;
	 if (setsockopt(_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0)
	   throw std::runtime_error("UdpReceiver: can't set multicast group membership") ;
    }

    socklen_t lenmyaddr = sizeof(sockaddr_in);
    if (bind(_socket, (struct sockaddr *)&name, lenmyaddr) < 0)
	 throw std::runtime_error("UdpReceiver: bind failed") ;

    for (unsigned int i=30; i>0; --i) {
	 _rcvbufsize = 1 << i ;
	 int res = setsockopt(_socket, SOL_SOCKET, SO_RCVBUF,
					  (char *)&_rcvbufsize, sizeof(_rcvbufsize)) ;
	 if (res!=-1) break ;
    }
    // std::cerr << "_rcvbufsize=" << _rcvbufsize << std::endl ;

    if (getsockname(_socket, (sockaddr *)&name, &lenmyaddr)!=-1)
	 _port = ntohs(name.sin_port) ; 
    else
	 _port = port ;

    _fromlen = sizeof(_from);

    _fk = FileKeeper::create(_socket, FileKeeper::R) ;
    subscribeTo(_fk) ;
  }

  void
  UdpReceiver::react(Observable *obs) {
    if (obs==_fk && (_fk->getState()&FileKeeper::R))
	 notifyObservers() ;
  }

  bool
  UdpReceiver::setBufferSize(int size) {
    int res = setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size)) ;
    return (res!=-1) ;
  }

  bool
  UdpReceiver::receive(unsigned char **data, unsigned int *size) {
    *data = new unsigned char [_rcvbufsize] ;
    int bytesRead = recvfrom(_socket, *data, _rcvbufsize, 0,
					    (struct sockaddr *)&_from, &_fromlen) ;
    if (bytesRead<1) {
	 delete [] *data ;
	 return false ;
    }
    *size = bytesRead ;
    return true ;
  }

  UdpReceiver::~UdpReceiver(void) {
    unsubscribeFrom(_fk) ;
    delete _fk ;
    shutdown(_socket,2) ;
    ::close(_socket) ;
  }

  // ------------------------------------------------------------------

}
