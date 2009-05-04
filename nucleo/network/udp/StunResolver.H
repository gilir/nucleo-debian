#ifndef n_StunResolver_H
#define n_StunResolver_H

/*
 *
 * nucleo/network/udp/StunResolver.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveObject.H>
#include <nucleo/core/TimeKeeper.H>
#include <nucleo/network/udp/UdpSocket.H>

namespace nucleo {

  class StunResolver : public ReactiveObject {

  private:

    // e.g. stunserver.org, stun.xten.com or stun.fwdnet.net
    std::list<std::string> servers ;
  
  public:

    StunResolver(void) ;
    StunResolver(const char *server) ;
    StunResolver(const char **servers, unsigned int nbServers) ;
    ~StunResolver(void) ;
    
    bool resolve(UdpSocket *socket, std::string *host, int *port, 
			  unsigned long timeout=10*TimeKeeper::second) ;

  } ;

}

#endif
