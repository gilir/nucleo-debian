/*
 *
 * nucleo/network/NetworkUtils.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/network/NetworkUtils.H>

#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <string>
#include <stdexcept>

namespace nucleo {

  in_addr_t
  resolveAddress(const char *addressOrHostname) {
    // First try it as aaa.bbb.ccc.ddd
    in_addr_t result = inet_addr(addressOrHostname) ;
    if (result != INADDR_NONE) return result ;

    struct hostent *host = gethostbyname(addressOrHostname) ;
    if (host==NULL) {
	 std::string msg = "resolveAddress: unable to resolve " ;
	 msg = msg + addressOrHostname ;
	 throw std::runtime_error(msg) ;
    }

    return (in_addr_t) *(in_addr_t*)host->h_addr ;
  }

  std::string
  getHostName(void) {
    char buffer[256] ;
    gethostname(buffer,256) ;
    return buffer ;
  }

}
