/*
 *
 * nucleo/network/udp/UdpPlusReceiver.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/udp/UdpPlusReceiver.H>

#include <nucleo/utils/FileUtils.H>

#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <arpa/inet.h>

#include <string>
#include <stdexcept>
#include <iostream>
#include <cstring>

namespace nucleo {

#define DEBUG_LEVEL 0

  // ---------------------------------------------------------------------------------

#if DEBUG_LEVEL>0
  static const char *UdpPlusReceiverStateName[] = {
    "OUT_OF_SYNC",
    "INCOMPLETE",
    "READY"
  } ;
#endif
	
  // ---------------------------------------------------------------------------------

  void
  UdpPlusReceiver::_open(const int port, const char *mcastGroup) {
    _bufferSize = UdpPlus::FragmentSize ;
    _buffer = new unsigned char [_bufferSize] ;

    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socket<0) throw std::runtime_error("UdpPlusReceiver: can't create socket") ;

    int rcvbuf ;
    for (unsigned int i=30; i>0; --i) {
	 rcvbuf = 1 << i ;
	 int res = setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, sizeof(rcvbuf)) ;
	 if (res!=-1) break ;
    }

#if DEBUG_LEVEL>0
    std::cerr << "UdpPlusReceiver: SO_RCVBUF=" << rcvbuf << " FragmentSize=" << UdpPlus::FragmentSize << std::endl ;
#endif

    if (mcastGroup) {
	 int one=1 ;
#ifdef SO_REUSEPORT
	 setsockopt(_socket, SOL_SOCKET, SO_REUSEPORT, (char *)&one, sizeof(int)) ;
#endif
	 setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(int)) ;

	 struct ip_mreq mreq ;
	 mreq.imr_multiaddr.s_addr = inet_addr(mcastGroup) ;
	 mreq.imr_interface.s_addr = htonl(INADDR_ANY) ;
	 if (setsockopt(_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0)
	   throw std::runtime_error("UdpPlusReceiver: can't set multicast group membership") ;
    }
  
    struct sockaddr_in name ;  
    memset(&name, 0, sizeof(name)) ;
    name.sin_family = AF_INET ;
    name.sin_addr.s_addr = htonl(INADDR_ANY) ;
    name.sin_port = htons(port) ;

    socklen_t lenmyaddr = sizeof(sockaddr_in);
    if (bind(_socket, (struct sockaddr *)&name, lenmyaddr) < 0)
	 throw std::runtime_error("UdpPlusReceiver: bind failed") ;

    if (getsockname(_socket, (sockaddr *)&name, &lenmyaddr)!=-1)
	 _port = name.sin_port ; 

    _state = OUT_OF_SYNC ;

    _fk = FileKeeper::create(_socket, FileKeeper::R) ;
    subscribeTo(_fk) ;
  }

  // ---------------------------------------------------------------------------------

  void
  UdpPlusReceiver::react(Observable *obs) {
    if (_state==OUT_OF_SYNC) _bytesRead=0 ;

    struct msghdr msg ;
    memset(&msg, 0, sizeof (msg)) ;
    struct iovec iov[2] ;
    iov[0].iov_base = (char *)&_header ;
    iov[0].iov_len = sizeof(_header) ;
    iov[1].iov_base = (char *)(_buffer+_bytesRead) ; 
    iov[1].iov_len = UdpPlus::FragmentSize ;
    msg.msg_iov = iov ;
    msg.msg_iovlen = 2 ;

    ssize_t s = recvmsg(_socket, &msg, 0) ;
    if (s<1) {
	 std::cerr << "UdpPlusReceiver::react: "
			   << strerror(errno) << std::endl ;
	 return ;
    }
    unsigned int fragmentSize = s-sizeof(_header) ;

#if DEBUG_LEVEL>0
    std::string dbg_prefix = "" ;
#endif

    if ( !_header.fnum) {
	 // Syncing on a new unit, forgetting the previous one
	 if (_bufferSize<_header.totalsize) {
	   unsigned int l = UdpPlus::FragmentSize + _header.totalsize ;
	   unsigned char *tmp = new unsigned char [l] ;
#if DEBUG_LEVEL>0
	   std::cerr << "Realloc: " << _bufferSize << " -> " << l << " bytes (" << fragmentSize << ")" << std::endl ;
#endif
	   memmove(tmp, iov[1].iov_base, fragmentSize) ;
	   delete [] _buffer ;
	   _buffer = tmp ;
	   _bufferSize = l ;
	 } else if (_state==INCOMPLETE) {
	   // The fragment went into the wrong place. Move it to the
	   // beginning of the buffer.
#if DEBUG_LEVEL>0
	   std::cerr << "Re-syncing on first fragment of unit #" << _header.unum << std::endl ;
#endif
	   memmove(_buffer, iov[1].iov_base, fragmentSize) ;
	 }

	 _unum = _header.unum ;
	 _fnum = 1 ;
	 _bytesRead = fragmentSize ;
	 _state = (_bytesRead==_header.totalsize) ? READY : INCOMPLETE ;
#if DEBUG_LEVEL>1
	 dbg_prefix = "S " ;
#endif
    } else if (_state==INCOMPLETE && _header.unum==_unum && _header.fnum==_fnum) {
	 // This is fragment is the one we were waiting for
	 _fnum++ ;
	 _bytesRead += fragmentSize ;
	 _state = (_bytesRead==_header.totalsize) ? READY : INCOMPLETE ;
#if DEBUG_LEVEL>1
	 dbg_prefix = "S " ;
#endif
    } else {
	 // This fragment is not the one we were waiting for, ignore it
#if DEBUG_LEVEL>0
	 dbg_prefix = "I " ;
#endif
    }

#if DEBUG_LEVEL>0
    if (dbg_prefix!="") {
	 std::cerr << dbg_prefix << _header.unum << "/" << _header.fnum << ": " ;
	 std::cerr << _bytesRead << "/" << _header.totalsize ;
	 std::cerr << " (" << _bufferSize << ") " ;
	 std::cerr << UdpPlusReceiverStateName[(int)_state] << std::endl ;
    }
#endif
    
    if (_state==READY) notifyObservers() ;
  }

  // ---------------------------------------------------------------------------------

  bool
  UdpPlusReceiver::receive(unsigned char **data, unsigned int *size) {
    if (_state!=READY) return false ;

    *data = _buffer ;
    *size = _bytesRead ;

    _state = OUT_OF_SYNC ;
    _buffer = new unsigned char [_bufferSize] ;

    return true ;
  }

  // ---------------------------------------------------------------------------------

  UdpPlusReceiver::~UdpPlusReceiver() {
    unsubscribeFrom(_fk) ;
    delete _fk ;
    shutdown(_socket,2) ;
    close(_socket) ;
  }

  // ---------------------------------------------------------------------------------

}
