#ifndef n_UdpSocket_H
#define n_UdpSocket_H

/*
 *
 * nucleo/network/udp/UdpSocket.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveObject.H>
#include <nucleo/core/FileKeeper.H>
#include <nucleo/core/URI.H>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace nucleo {

  class UdpSocket : public ReactiveObject {

  protected:

    int protocol, socket ;
    FileKeeper *signal ;

    void react(Observable *obs) ;

  public:

    UdpSocket(int protocol=PF_INET) ;
    ~UdpSocket(void) ;

    int getDescriptor(void) { return socket ; }
    int getProtocol(void) { return protocol ; }
    int getPortNumber(void) ;
    bool getPeerName(sockaddr_storage *ss) ;

    bool setBufferSizes(int ssize, int rsize) ;
    int getSendBufferSize(void) ;
    int getReceiveBufferSize(void) ;

    bool listenTo(int port, const char *mcastGroup=0) ;
    ssize_t receive(void *buffer, size_t bufferSize, sockaddr_storage *ss=0) ;
    ssize_t receive(iovec *iov, u_int iovlen, sockaddr_storage *ss=0) ;

    bool connectTo(const char *host, int port) ;
    bool connectTo(const char *host, const char *port) ;
    bool resolve(const char *host, int port, sockaddr_storage *ss) ;
    bool resolve(const char *host, const char *port, sockaddr_storage *ss) ;
    ssize_t send(const void *data, size_t size, sockaddr_storage *ss=0) ;
    ssize_t send(iovec *iov, u_int iovlen, sockaddr_storage *ss=0) ;
    bool disconnect(void) ;

    void setTTL(unsigned char ttl) ; // 1=subnet, 31=site, 64=national, >=127=worldwide
    void setLoopback(bool on) ;

  } ;

  bool sockaddr2hostport(sockaddr_storage *ss, std::string *host, std::string *port) ;
  bool sockaddr2hostport(sockaddr_storage *ss, std::string *host, int *port) ;

  std::string sockaddr2string(sockaddr_storage *ss) ;

  sockaddr_storage *sockaddrdup(sockaddr_storage *ss) ;

}

#endif
