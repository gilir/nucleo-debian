/*
 *
 * nucleo/network/tcp/TcpConnection.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/network/NetworkUtils.H>
#include <nucleo/network/tcp/TcpConnection.H>
#include <nucleo/network/tcp/TcpUtils.H>
#include <nucleo/utils/FileUtils.H>
#include <nucleo/core/FileKeeper.H>
#include <nucleo/core/TimeKeeper.H>
#include <nucleo/core/ReactiveEngine.H>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>
#include <cstdio>
#include <errno.h>
#include <sys/errno.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>

namespace nucleo {

#define VERBOSE_CONNECT 0

  // -------------------------------------------------------------------------

  void
  TcpConnection::connectTo(in_addr_t address, int port) {
    _socket = socket(AF_INET, SOCK_STREAM, 0) ;
    if (_socket==-1)
	 throw std::runtime_error("TcpConnection: unable to create socket") ;

    struct sockaddr_in addr ;
    memset(&addr, 0, sizeof(addr)) ;
    addr.sin_family = AF_INET ;
    addr.sin_port = htons(port) ;
    addr.sin_addr.s_addr = address ;

#if VERBOSE_CONNECT
    unsigned char *tmp = (unsigned char *)&address ;
    std::cerr << "Connecting " << this << " to "
		    << (int)tmp[0] << "."<< (int)tmp[1] << "." << (int)tmp[2] << "." << (int)tmp[3]
		    << ":" << port
		    << std::endl ;
#endif

    bool connected = false ;
    setblocking(_socket, 0) ;

    if (!connect(_socket, (sockaddr *)&addr, sizeof(addr))) {
	 connected = true ;
    } else if (errno==EINPROGRESS) {
	 FileKeeper *fk = FileKeeper::create(_socket, FileKeeper::W) ;
	 TimeKeeper *tk = TimeKeeper::create(3000) ; // 3 seconds...
	 WatchDog rantanplan(tk) ;
	 do {
	   ReactiveEngine::step() ;
#if VERBOSE_CONNECT
	   std::cerr << "fk: " << fk->getState() << " (" << FileKeeper::W << ")" << std::endl ;
#endif
	   connected = fk->getState()&FileKeeper::W ;
	   if (rantanplan.sawSomething()) {
#if VERBOSE_CONNECT
		std::cerr << "timed out!" << std::endl ;
#endif
		break ;
	   }
	 } while (!connected) ;
	 delete tk ;
	 delete fk ;
    }

#if VERBOSE_CONNECT
	 int val=0 ;
	 socklen_t len=sizeof(val) ;
	 getsockopt(_socket,SOL_SOCKET,SO_ERROR,&val,&len) ;
	 std::cerr << "connected=" << connected << " SO_ERROR=" << val << std::endl ;
#endif

    setblocking(_socket, 1) ;
    if (!connected) {
	 shutdown(_socket,2) ;
	 ::close(_socket) ;
	 throw std::runtime_error("TcpConnection: failed to connect") ;
    }

    _closeSocketOnDestroy = true ;
    setDefaultTcpSocketOptions(_socket, false) ; 
    _tcpw = FileKeeper::create(_socket, FileKeeper::R) ;
    subscribeTo(_tcpw) ;
    // std::cerr << "TcpConnection " << this << " uses FileKeeper " << _tcpw << std::endl ;
  }

  // -------------------------------------------------------------------------

  TcpConnection::TcpConnection(int socket, bool close) {
    // std::cerr << "Creating TcpConnection " << this << " from socket" << std::endl ;
    if (socket==-1) throw std::runtime_error("TcpConnection: bad socket (-1)") ;
    _socket = socket ;

    _closeSocketOnDestroy = close ;
    setDefaultTcpSocketOptions(_socket, false) ; 
    _tcpw = FileKeeper::create(_socket, FileKeeper::R) ;
    subscribeTo(_tcpw) ;
    // std::cerr << "TcpConnection " << this << " uses FileKeeper " << _tcpw << std::endl ;
  }    

  TcpConnection::TcpConnection(in_addr_t address, int port) {
    // std::cerr << "Creating TcpConnection " << this << " from address/port" << std::endl ;
    connectTo(address, port) ;
  }

  TcpConnection::TcpConnection(const char* hostname, int port) {
    // std::cerr << "Creating TcpConnection " << this << " from hostname/port" << std::endl ;
    in_addr_t address = resolveAddress(hostname) ;
    connectTo(address, port) ;
  }

  TcpConnection::TcpConnection(std::string hostname, int port) {
    // std::cerr << "Creating TcpConnection " << this << " from hostname/port" << std::endl ;
    in_addr_t address = resolveAddress(hostname.c_str()) ;
    connectTo(address, port) ;
  }

  // -------------------------------------------------------------------------

  unsigned int
  TcpConnection::send(const char *data, unsigned int length, bool complete) {
    if (!length) return 0 ;

    int n ;
    unsigned int bytesToWrite = length ;
    char *ptr = (char *)data ;
    do {
	 n = write(_socket, ptr, bytesToWrite) ;
	 if (n==-1) {
	   if (errno!=EAGAIN) {
		std::string errorMessage = "TcpConnection: write failed " ;
		errorMessage = errorMessage + "(" + strerror(errno) + ")" ;
		throw std::runtime_error(errorMessage) ;
	   }
	 } else {
	   bytesToWrite -= n ;
	   ptr += n ;
	 }
    } while (n && bytesToWrite && complete) ;

    return length-bytesToWrite ;
  }

  // -------------------------------------------------------------------------

  unsigned int
  TcpConnection::receive(char *data, unsigned int length, bool complete) {
    if (!data || !length) return 0 ;

    int n ;
    unsigned int bytesToRead = length ;
    char *ptr = data ;
    do {
	 n = read(_socket, (void *)ptr, bytesToRead) ;
	 // std::cerr << "TcpConnection::receive -> " << n << " bytes (" << length << ")" << std::endl ;
	 if (n==-1) {
	   if (errno!=EAGAIN) {
		std::string errorMessage = "TcpConnection: read failed " ;
		errorMessage = errorMessage + "(" + strerror(errno) + ")" ;
		throw std::runtime_error(errorMessage) ;
	   }
	 } else {
	   bytesToRead -= n ;
	   ptr += n ;
	 }
    } while (bytesToRead && n && complete) ;

    return length-bytesToRead ;
  }

  // -------------------------------------------------------------------------

  std::string
  TcpConnection::machineLookUp(int *port) {
    return getRemoteTcpHost(_socket, port) ;
  }

  // -------------------------------------------------------------------------

  std::string
  TcpConnection::userLookUp(int millisecs) {
    sockaddr_in myaddr;
    socklen_t lenmyaddr = sizeof(sockaddr_in);
    if (getsockname(_socket, (sockaddr *)&myaddr, &lenmyaddr)==-1) return "?" ;

    sockaddr_in hisaddr;
    socklen_t lenhisaddr = sizeof(sockaddr_in);
    if (getpeername(_socket, (sockaddr *)&hisaddr, &lenhisaddr)==-1) return "?" ;

    struct hostent *h = gethostbyaddr((char *)&hisaddr.sin_addr, sizeof(hisaddr.sin_addr),
							   AF_INET) ;
    if (h==0) return "?" ;

    try {
	 TcpConnection identd(*(in_addr_t*)h->h_addr, 113) ;
	 char buffer[512] ;
	 sprintf(buffer, "%d, %d\n", ntohs(hisaddr.sin_port), ntohs(myaddr.sin_port)) ;

	 identd.send(buffer, strlen(buffer)) ;

	 unsigned int length = identd.receive(buffer, 512, false) ;

	 while (length>0 && isspace((int)buffer[length-1])) length-- ;
	 buffer[length] = '\0';
	 while (length>0 && buffer[length-1]!=':' && !isspace((int)buffer[length-1])) length-- ;
	 return std::string(buffer+length) ;
    } catch (std::runtime_error e) {
	 std::cerr << e.what() << std::endl ;
    }

    return "?" ;
  }

  // -------------------------------------------------------------------------

  TcpConnection::~TcpConnection(void) {
    unsubscribeFrom(_tcpw) ;
    delete _tcpw ;
    if (_closeSocketOnDestroy) {
	 shutdown(_socket,2) ;
	 ::close(_socket) ;
    }
  }

  // -------------------------------------------------------------------------

}
