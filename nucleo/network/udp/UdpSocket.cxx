/*
 *
 * nucleo/network/udp/UdpSocket.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/udp/UdpSocket.H>
#include <nucleo/network/NetworkUtils.H>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/errno.h>

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>

namespace nucleo {

  // http://en.wikipedia.org/wiki/IPv4
  // http://en.wikipedia.org/wiki/Ipv6
  // http://gsyc.escet.urjc.es/~eva/IPv6-web/ipv6.html

  // --------------------------------------------------------------------------

  UdpSocket::UdpSocket(int proto) {
    if (proto!=PF_INET && proto!=PF_INET6)
	 throw std::runtime_error("UdpSocket: unsupported protocol (use PF_INET or PF_INET6)") ;
    protocol = proto ;
    socket = ::socket(protocol, SOCK_DGRAM, 0) ;
    if (socket<0)
	 throw std::runtime_error("UdpSocket: can't create socket") ;
    signal = 0 ;
    setBufferSizes(-1,-1) ;
  }

  UdpSocket::~UdpSocket(void) {
    if (signal) {
	 unsubscribeFrom(signal) ;
	 delete signal ;
    }
    shutdown(socket,SHUT_RDWR) ;
    close(socket) ;
  }

  // --------------------------------------------------------------------------

  bool
  UdpSocket::setBufferSizes(int ssize, int rsize) {
    bool result = true ;
    if (rsize) {
	 if (rsize<0) {
	   for (unsigned int i=30; i>0; --i) {
		rsize = 1 << i ;
		if (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char *)&rsize, sizeof(int))!=-1)
		  break ;
	   }
	 } else {
	   int error = setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char *)&rsize, sizeof(int)) ;
	   result = result && (error!=-1) ;
	 }
    }
    if (ssize) {
	 if (ssize<0) {
	   for (unsigned int i=30; i>0; --i) {
		ssize = 1 << i ;
		if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *)&ssize, sizeof(int))!=-1)
		  break ;
	   }
	 } else {
	   int error = setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *)&ssize, sizeof(int)) ;
	   result = result && (error!=-1) ;
	 }
    }
    return result ;
  }

  int
  UdpSocket::getReceiveBufferSize(void) {
    int value = 0 ;
    socklen_t length = sizeof(int) ;
    getsockopt(socket,SOL_SOCKET,SO_RCVBUF,&value,&length) ;
    return value ;
  }

  int
  UdpSocket::getSendBufferSize(void) {
    int value = 0 ;
    socklen_t length = sizeof(int) ;
    getsockopt(socket,SOL_SOCKET,SO_SNDBUF,&value,&length) ;
    return value ;
  }

  int
  UdpSocket::getPortNumber(void) {
    sockaddr_storage ss ;  
    socklen_t sslen = sizeof(sockaddr_storage) ;
    if (getsockname(socket, (sockaddr *)&ss, &sslen)==-1) 
	 return 0 ;
    if (ss.ss_family==AF_INET) {
	 sockaddr_in *sin = (sockaddr_in *)&ss ;
	 return ntohs(sin->sin_port) ;
    } else if (ss.ss_family==AF_INET6) {
	 sockaddr_in6 *sin6 = (sockaddr_in6 *)&ss ;
	 return ntohs(sin6->sin6_port) ;
    } else
	 return 0 ;
  }

  bool
  UdpSocket::getPeerName(sockaddr_storage *ss) {
    memset(ss, 0, sizeof(sockaddr_storage)) ;
    socklen_t sslen = sizeof(sockaddr_storage) ;
    return (getpeername(socket, (sockaddr*)ss, &sslen)==0) ;
  }

  // --------------------------------------------------------------------------

  bool
  UdpSocket::listenTo(int port, const char *mcastGroup) {
    sockaddr_storage ss ;
    memset(&ss, 0, sizeof(sockaddr_storage)) ;
    if (protocol==PF_INET6) {
	 sockaddr_in6 *addrv6 = (sockaddr_in6*)&ss ;
	 ss.ss_family = AF_INET6 ;
	 addrv6->sin6_addr = in6addr_any ;
	 addrv6->sin6_port = htons(port) ;
    } else {
	 sockaddr_in *addrv4 = (sockaddr_in*)&ss ;
	 ss.ss_family = AF_INET ;
	 addrv4->sin_addr.s_addr = INADDR_ANY ;
	 addrv4->sin_port = htons(port) ;
    }

    if (mcastGroup) { // Request that the kernel join a multicast group
	 if (protocol==PF_INET6) {
	   ipv6_mreq mreq ;
	   inet_pton(AF_INET6, mcastGroup, &mreq.ipv6mr_multiaddr) ;
	   if (IN6_IS_ADDR_MULTICAST(&mreq.ipv6mr_multiaddr)) {
		mreq.ipv6mr_interface = 0 ;
		if (setsockopt(socket,IPPROTO_IPV6,IPV6_JOIN_GROUP,&mreq,sizeof(ipv6_mreq)) < 0)
		  throw std::runtime_error("UdpSocket: can't set IPv6 multicast group membership") ;
	   } else 
		std::cerr << "UdpSocket warning: '" << mcastGroup << "' is not a valid IPv6 multicast group" << std::endl ;
	 } else {
	   ip_mreq mreq ;
	   mreq.imr_multiaddr.s_addr = inet_addr(mcastGroup) ;
	   if (IN_MULTICAST(ntohl(mreq.imr_multiaddr.s_addr))) {
		mreq.imr_interface.s_addr = 0 ;
		if (setsockopt(socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(ip_mreq)) < 0)
		  throw std::runtime_error("UdpSocket: can't set IPv4 multicast group membership") ;
	   } else
		std::cerr << "UdpSocket warning: '" << mcastGroup << "' is not a valid IPv4 multicast group" << std::endl ;
	 }
	 int one=1 ;
	 setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) ;
#ifdef SO_REUSEPORT
	 setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(int)) ;
#endif
    }

    if (bind(socket, (sockaddr *)&ss, ss.ss_family==PF_INET6?sizeof(sockaddr_in6):sizeof(sockaddr_in)) < 0)
	 return false ;

    signal = FileKeeper::create(socket, FileKeeper::R) ;
    subscribeTo(signal) ;
    return true ;
  }

  void
  UdpSocket::react(Observable *obs) {
    if (obs==signal && (signal->getState()&FileKeeper::R))
	 notifyObservers() ;
  }

  ssize_t
  UdpSocket::receive(void *data, size_t size, sockaddr_storage *addr) {
    sockaddr_storage tmp_addr ;  
    socklen_t addrlen = sizeof(sockaddr_storage) ;
    return recvfrom(socket, data, size, 0, 
				(sockaddr *)(addr?addr:&tmp_addr), &addrlen) ;
  }

  ssize_t
  UdpSocket::receive(iovec *iov, u_int iovlen, sockaddr_storage *addr) {
    sockaddr_storage tmp_addr ;  
    msghdr mh ;
    memset(&mh, 0, sizeof(msghdr)) ;
    mh.msg_name = addr ? addr : &tmp_addr ;
    mh.msg_namelen = sizeof(sockaddr_storage) ;
    mh.msg_iov = iov ;
    mh.msg_iovlen = iovlen ;
    return recvmsg(socket, &mh, 0) ;
  }

  // --------------------------------------------------------------------------

  bool
  UdpSocket::connectTo(const char *host, const char *port) {
    addrinfo *result, hints ;    
    memset(&hints, 0, sizeof(addrinfo)) ;
    hints.ai_family = protocol ;
    hints.ai_socktype = SOCK_DGRAM ;
    hints.ai_protocol = IPPROTO_UDP ;
    int error = getaddrinfo(host, port, &hints, &result) ;
    if (error) {
	 // std::cerr << "UdpSocket::connect: " << strerror(errno) << " (" << host << ":" << port << ")" << std::endl ;
	 return false ;
    }
    bool connected = false ;
    for (addrinfo *ptr=result; ptr; ptr=ptr->ai_next) {
	 int error = connect(socket, ptr->ai_addr, ptr->ai_addrlen) ;
	 if (!error) { connected = true ; break ; }
	 // std::cerr << "UdpSocket::connect: " << strerror(errno) << " (" << host << ":" << port << ")" << std::endl ;
    }
    freeaddrinfo(result);
    return connected ;
  }

  bool
  UdpSocket::connectTo(const char *host, int port) {
    char tmp[512] ;
    snprintf(tmp, 512, "%d", port) ;
    return connectTo(host, tmp) ;
  }

  bool
  UdpSocket::resolve(const char *host, const char *port, sockaddr_storage *ss) {
    memset(ss, 0, sizeof(sockaddr_storage)) ;
    addrinfo *result, hints ;    
    memset(&hints, 0, sizeof(addrinfo)) ;
    hints.ai_family = protocol ; // or PF_UNSPEC?
    hints.ai_socktype = SOCK_DGRAM ;
    hints.ai_protocol = IPPROTO_UDP ;
    int error = getaddrinfo(host, port, &hints, &result) ;
    if (error || !result) {
	 // std::cerr << "UdpSocket::resolve: " << strerror(errno) << " (" << host << ":" << port << ")" << std::endl ;
	 return false ;
    }
    memcpy(ss,result->ai_addr,result->ai_addrlen) ;
    freeaddrinfo(result);
    if (ss->ss_family!=PF_INET && ss->ss_family!=PF_INET6) {
	 std::cerr << "UdpSocket::resolve: unsupported protocol (" << ss->ss_family << ")" << std::endl ;
	 return false ;
    }

    if (ss->ss_family!=protocol)
	 std::cerr << "UdpSocket::resolve warning: protocol mismatch" << std::endl ;

    return true ;
  }

  bool
  UdpSocket::resolve(const char *host, int port, sockaddr_storage *ss) {
    char tmp[512] ;
    snprintf(tmp, 512, "%d", port) ;
    return resolve(host, tmp, ss) ;   
  }

  ssize_t
  UdpSocket::send(const void *data, size_t size, sockaddr_storage *ss) {
    if (ss) {
	 return sendto(socket, data, size, 0, (sockaddr*)ss, ss->ss_family==PF_INET6?sizeof(sockaddr_in6):sizeof(sockaddr_in)) ;
    } else
	 return ::send(socket, data, size, 0) ;
  }

  ssize_t
  UdpSocket::send(iovec *iov, u_int iovlen, sockaddr_storage *ss) {
#if 0
    sockaddr_storage tmp ;
    if (!ss) { getPeerName(&tmp) ; ss = &tmp ; }
#endif
    msghdr mh ;
    memset(&mh, 0, sizeof(msghdr)) ;
    mh.msg_name = ss ;
    mh.msg_namelen = ss ? (ss->ss_family==PF_INET6?sizeof(sockaddr_in6):sizeof(sockaddr_in)) : sizeof(sockaddr_storage) ;
    mh.msg_iov = iov ;
    mh.msg_iovlen = iovlen ;
    return sendmsg(socket, &mh, 0) ;
  }

  bool
  UdpSocket::disconnect(void) {
    if (protocol==PF_INET6) {
	 sockaddr_in6 sa ;
	 memset(&sa, 0, sizeof(sockaddr_in6)) ;
	 sa.sin6_family = AF_UNSPEC ;
	 int error = connect(socket, (sockaddr*)&sa, sizeof(sockaddr_in6)) ;
	 if (error && errno!=EAFNOSUPPORT)  {
	   std::cerr << "UdpSocket::disconnect (IPv6): " << strerror(errno) << std::endl ;
	   return false ;
	 }
    } else {
	 sockaddr_in sa ;
	 memset(&sa, 0, sizeof(sockaddr_in)) ;
	 sa.sin_family = AF_UNSPEC ;
	 int error = connect(socket, (sockaddr*)&sa, sizeof(sockaddr_in)) ;
	 if (error && errno!=EAFNOSUPPORT)  {
	   std::cerr << "UdpSocket::disconnect (IPv4): " << strerror(errno) << std::endl ;
	   return false ;
	 }
    }
    return true ;
  }

  // --------------------------------------------------------------------------

  void
  UdpSocket::setTTL(unsigned char ttl) { 
    int level = IPPROTO_IP, optname = IP_MULTICAST_TTL ;
    if (protocol==PF_INET6) {
	 level = IPPROTO_IPV6 ;
	 optname = IPV6_MULTICAST_HOPS ;
    }
    if (setsockopt(socket, level, optname, &ttl, 1) == -1)
	 throw std::runtime_error("UdpSocket: can't set TTL") ;
  }

  void
  UdpSocket::setLoopback(bool on) {
    int value = on ? 1 : 0 ;
    int level = IPPROTO_IP, optname = IP_MULTICAST_TTL ;
    if (protocol==PF_INET6) {
	 level = IPPROTO_IPV6 ;
	 optname = IPV6_MULTICAST_LOOP ;
    }
    if (setsockopt(socket, level, optname, &value, sizeof(int)) == -1)
	 throw std::runtime_error("UdpSocket: can't set loopback mode") ;
  }

  // --------------------------------------------------------------------------

  bool
  sockaddr2hostport(sockaddr_storage *ss, std::string *host, std::string *port) {
    char h[NI_MAXHOST] ;
    char p[NI_MAXSERV] ;
    if (getnameinfo((sockaddr*)ss, sizeof(sockaddr_storage),
				h, sizeof(h), p, sizeof(p), 
				NI_NUMERICHOST | NI_NUMERICSERV)< 0) 
	 return false ;
    if (host) *host = h ;
    if (port) *port = p ;
    return true ;
  }

  bool
  sockaddr2hostport(sockaddr_storage *ss, std::string *host, int *port) {
    std::string p ;
    if (!sockaddr2hostport(ss, host, &p)) return false ;
    if (port && !p.empty()) *port = strtol(p.c_str(),0,10) ;
    return true ;
  }

  std::string
  sockaddr2string(sockaddr_storage *ss) {
    char hostname[NI_MAXHOST] ;
    char service[NI_MAXSERV] ;
    int error = getnameinfo((sockaddr*)ss, sizeof(sockaddr_storage),
					   hostname, sizeof(hostname), service, sizeof(service), 
					   NI_NUMERICHOST | NI_NUMERICSERV) ;
    std::stringstream result ;
    if (error)
	 result << "<" << gai_strerror(error) << ">" ;
    else if (ss->ss_family==AF_INET6 && !IN6_IS_ADDR_V4MAPPED(&((sockaddr_in6 *)ss)->sin6_addr))
	 result << "[" << hostname << "]:" << service ;
    else
	 result << hostname << ":" << service ;
    return result.str() ;
  }

  sockaddr_storage *
  sockaddrdup(sockaddr_storage *ss) {
    if (!ss) return 0 ;
    sockaddr_storage *sscopy = new sockaddr_storage ;
    bcopy(ss,sscopy,sizeof(sockaddr_storage)) ;
    return sscopy ;
  }

}
