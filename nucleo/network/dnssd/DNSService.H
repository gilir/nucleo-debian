#ifndef n_DNSService_H
#define n_DNSService_H

/*
 *
 * nucleo/network/dnssd/DNSService.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <string>
#include <map>

#include <stdint.h>

namespace nucleo {

  typedef std::map<std::string, std::string> DNSTextRecord ;

  struct DNSService {
    // Minimum service information
    std::string interface ;
    std::string type ;
    std::string name ;
    // Additional information (obtained when the service is resolved)
    std::string domain ;
    std::string host ;
    uint16_t port ;
    DNSTextRecord textRecord ;
  } ;

}
  
#endif
