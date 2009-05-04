#ifndef n_UdpPlusSender_H
#define n_UdpPlusSender_H

/*
 *
 * nucleo/network/udp/UdpPlusSender.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/udp/UdpPlus.H>

#include <nucleo/core/URI.H>

#include <sys/types.h>
#include <netinet/in.h>

namespace nucleo {

  // ------------------------------------------------------

  class UdpPlusSender {

  private:

    int _socket ;
    unsigned int _unum ;
    struct sockaddr_in _peer ;

  public:

    UdpPlusSender(const char *hostOrGroup, int port) ;

    void setTTL(const unsigned char ttl) ;
    bool send(const void *content, unsigned int contentLength) ;

    ~UdpPlusSender(void) ;

  } ;

  // ------------------------------------------------------

}

#endif
