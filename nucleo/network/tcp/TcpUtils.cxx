/*
 *
 * nucleo/network/tcp/TcpUtils.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/network/tcp/TcpUtils.H>

#include <iostream>
#include <stdexcept>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/time.h>
#include <arpa/inet.h> 

#include <signal.h>

namespace nucleo {

  void
  setDefaultTcpSocketOptions(int sock, bool server) {
    int one=1 ;

    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&one, sizeof(int)) ;
#ifdef TCP_FASTACK
    setsockopt(sock, IPPROTO_TCP, TCP_FASTACK, (void *)&one, sizeof(int)) ;
#endif

    struct timeval one_second = {1,0} ;
#ifdef SO_RCVTIMEO
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void *)&one_second, sizeof(timeval)) ;
#endif
#ifdef SO_SNDTIMEO
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (void *)&one_second, sizeof(timeval)) ;
#endif

    if (server) {
	 // so we can restart the server later without having to wait for
	 // the system to release the port
#ifdef SO_REUSEPORT
	 setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void *)&one, sizeof(int)) ;
#endif
	 setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&one, sizeof(int)) ;
    }

#ifdef SO_NOSIGPIPE
    setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&one, sizeof(int)) ;
#else  
    signal(SIGPIPE, SIG_IGN) ;
#endif
  }

  std::string
  getRemoteTcpHost(int socket, int *port) {
    struct sockaddr sa ;
    socklen_t salen = sizeof(sa) ;
    int error = getpeername(socket, (struct sockaddr *)&sa, &salen) ;
    if (error) throw std::runtime_error("getRemoteTcpHost: getpeername failed") ;

    char addr[NI_MAXHOST];
    error = getnameinfo(&sa, salen, addr, sizeof(addr), NULL, 0, NI_NAMEREQD);
    if (!error) {
	 struct addrinfo hints, *res;
	 memset(&hints, 0, sizeof(hints));
	 hints.ai_socktype = SOCK_DGRAM; /*dummy*/
	 hints.ai_flags = AI_NUMERICHOST;
	 if (getaddrinfo(addr, "0", &hints, &res) == 0) {
	   freeaddrinfo(res);
	   throw std::runtime_error("TcpUtils::getRemoteTcpHost: bogus PTR record (malicious record?)");
	 }
	 // addr is FQDN as a result of PTR lookup
	 return addr ;
    }
    
    // addr is numeric string
    error = getnameinfo(&sa, salen, addr, sizeof(addr), NULL, 0, NI_NUMERICHOST);
    return addr ;
  }
  
}
