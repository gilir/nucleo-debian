#ifndef n_DNSServiceAnnouncer_H
#define n_DNSServiceAnnouncer_H
    
/*
 *
 * nucleo/network/dnssd/DNSServiceAnnouncer.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/nucleo.H>
#include <nucleo/network/dnssd/DNSService.H>

#if NUCLEO_SUPPORTS_DNS_SD
#include <arpa/inet.h>
#include <dns_sd.h>
#endif

#include <stdexcept>
#include <sstream>
#include <iostream>

namespace nucleo {

  class DNSServiceAnnouncer {

  public:

    typedef enum {NOERROR, ERROR, NAMECONFLICT} state ;

  protected:

    std::string name, regtype, domain ;
    std::string host ;
    uint16_t port ;
    std::string textRecords ;
    state status ;

#if NUCLEO_SUPPORTS_DNS_SD
    DNSServiceRef sdRef ;

    static void registerCallback(DNSServiceRef sdRef,
						   DNSServiceFlags flags,
						   DNSServiceErrorType errorCode,
						   const char *name, const char *regtype,
						   const char *domain,
						   void *context) ;
#endif

  public:

    DNSServiceAnnouncer(void) ;
    DNSServiceAnnouncer(const char *name, const char *regtype, uint16_t port,
				    DNSTextRecord *textrecord=0, 
				    const char *host=0, const char *domain=0) ;

    bool announce(const char *name, const char *regtype, uint16_t port,
			   DNSTextRecord *textrecord=0, 
			   const char *host=0, const char *domain=0) ;

    std::string getName(void) { return name ; }
    state getStatus(void) { return status ; }

    ~DNSServiceAnnouncer(void) ;

  } ;

}

#endif
