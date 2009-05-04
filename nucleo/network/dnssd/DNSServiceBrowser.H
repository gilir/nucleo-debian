#ifndef n_DNSServiceBrowser_H
#define n_DNSServiceBrowser_H

/*
 *
 * nucleo/network/dnssd/DNSServiceBrowser.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/nucleo.H>

#include <nucleo/core/ReactiveObject.H>
#include <nucleo/network/dnssd/DNSService.H>
#include <nucleo/core/FileKeeper.H>

#if NUCLEO_SUPPORTS_DNS_SD
#include <arpa/inet.h>
#include <dns_sd.h>
#include <sys/socket.h>
#include <net/if.h>
#endif

#include <queue>

namespace nucleo {

  class DNSServiceBrowser : public ReactiveObject {

  public:

    struct Event {
	 enum {FOUND, LOST} event ;
	 DNSService service ;
	 void debug(std::ostream& out) const ;
    } ;

  protected:

    FileKeeper *watchdog ;
    std::queue<Event*> queue ;

#if NUCLEO_SUPPORTS_DNS_SD
    DNSServiceRef sdRef ;

    static void resolveCallBack(DNSServiceRef sdRef,
						  DNSServiceFlags flags,
						  uint32_t interfaceIndex,
						  DNSServiceErrorType errorCode,
						  const char *fullname, const char *hosttarget,
						  uint16_t port,
						  uint16_t txtLen, const unsigned char *txtRecord,
						  void *context) ;

    static void browseCallBack(DNSServiceRef sdRef,
						 DNSServiceFlags flags,
						 uint32_t interfaceIndex,
						 DNSServiceErrorType errorCode,
						 const char *serviceName,
						 const char *replyType,
						 const char *replyDomain,
						 void *context) ;
#endif

    void react(Observable *obs) ;

  public:

    DNSServiceBrowser(void) ;
    DNSServiceBrowser(const char *regtype, const char *domain=0) ;

    void browse(const char *regtype, const char *domain=0) ;
    Event *getNextEvent(void) ;

    ~DNSServiceBrowser(void) ;

  } ;

}

#endif
