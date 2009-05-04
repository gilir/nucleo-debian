#ifndef n_UdpSender_H
#define n_UdpSender_H

/*
 *
 * nucleo/network/udp/UdpSender.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>

namespace nucleo {

  class UdpSender {

  private:

    int _fd ;
    struct sockaddr_in _peer ;

    void init(const char *hostname, int port) ;

  public:

    UdpSender(std::string hostname, int port) { init(hostname.c_str(), port) ; }
    UdpSender(const char *hostname, int port) { init(hostname, port) ; }

    // 1=subnet, 31=site, 64=national, 127 and more=worldwide
    void setMulticastTTL(unsigned int ttl) ;

    bool setBufferSize(int size) ;

    // returns the number of bytes sent
    int send(const void *content, unsigned int contentLength) ;
  
    ~UdpSender(void) ;

  } ;

}

#endif
