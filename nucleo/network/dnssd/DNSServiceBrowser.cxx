/*
 *
 * nucleo/network/dnssd/DNSServiceBrowser.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/dnssd/DNSServiceBrowser.H>

#include <stdexcept>
#include <sstream>
#include <iostream>

namespace nucleo {

  DNSServiceBrowser::DNSServiceBrowser(void) {
#if NUCLEO_SUPPORTS_DNS_SD
    sdRef = 0 ;
    watchdog = 0 ;
#endif
  }

  DNSServiceBrowser::DNSServiceBrowser(const char *regtype, const char *domain) {
#if NUCLEO_SUPPORTS_DNS_SD
    sdRef = 0 ;
    watchdog = 0 ;
    browse(regtype, domain) ;
#endif
  }

  void
  DNSServiceBrowser::browse(const char *regtype, const char *domain) {
#if NUCLEO_SUPPORTS_DNS_SD
    if (sdRef) {
	 DNSServiceRefDeallocate(sdRef) ;
	 sdRef = 0 ; 
	 unsubscribeFrom(watchdog) ;
	 delete watchdog ;
	 watchdog = 0 ;
    }

    DNSServiceErrorType result = DNSServiceBrowse(&sdRef,
										0, 0,
										regtype, domain,
										browseCallBack,
										(void *)this); 
    if (result!=kDNSServiceErr_NoError)
	 throw std::runtime_error("DNSServiceBrowse failed") ;

    watchdog = FileKeeper::create(DNSServiceRefSockFD(sdRef), FileKeeper::R) ;
    subscribeTo(watchdog) ;
#endif
  }

#if NUCLEO_SUPPORTS_DNS_SD
  void
  DNSServiceBrowser::resolveCallBack(DNSServiceRef sdRef,
							  DNSServiceFlags flags,
							  uint32_t interfaceIndex,
							  DNSServiceErrorType errorCode,
							  const char *fullname, const char *hosttarget,
							  uint16_t port,
							  uint16_t txtLen, const unsigned char *txtRecord,
							  void *context) {
    Event *e = (Event*)context ;
    e->service.host = hosttarget ;
    e->service.port = ntohs(port) ;
    
    uint16_t start=0, eq=0 ;
    while (start<txtLen) {
	 uint16_t recordLen = txtRecord[start] ;
	 if (recordLen>0) {
	   std::string key, value ;
	   for (eq=start+recordLen; eq && eq>=start && txtRecord[eq]!='='; --eq) ;
	   if (eq==start) {
		key.assign((const char*)txtRecord, start+1, recordLen) ;
	   } else if (eq==start+recordLen) {
		key.assign((const char*)txtRecord, start+1, recordLen-1) ;
	   } else {
		key.assign((const char*)txtRecord, start+1, eq-start-1) ;
		value.assign((const char*)txtRecord, eq+1, start+recordLen-eq) ;
	   }
	   e->service.textRecord[key] = value ;
	 }
	 start += recordLen+1 ;
    }
  }
  
  void
  DNSServiceBrowser::browseCallBack(DNSServiceRef sdRef,
							 DNSServiceFlags flags,
							 uint32_t interfaceIndex,
							 DNSServiceErrorType errorCode,
							 const char *serviceName,
							 const char *replyType,
							 const char *replyDomain,
							 void *context) {
    if (errorCode!=kDNSServiceErr_NoError) return ;

    DNSServiceBrowser *browser = (DNSServiceBrowser*)context ;

    Event *e = new Event ;
    e->service.name = serviceName ;
    e->service.type = replyType ;
    e->service.domain = replyDomain ;
    char interfaceName[IF_NAMESIZE*10] ;
    if (if_indextoname(interfaceIndex,interfaceName))
	 e->service.interface = interfaceName ;

    if (flags & kDNSServiceFlagsAdd) {
	 e->event = Event::FOUND ;
	 DNSServiceRef subref ; 	   
	 DNSServiceErrorType result = DNSServiceResolve(&subref, 0,
										   interfaceIndex,
										   serviceName,replyType,replyDomain,
										   (DNSServiceResolveReply)resolveCallBack,
										   (void *)e) ;
	 if (result!=kDNSServiceErr_NoError)
	   std::cerr << "DNSServiceResolve failed" << std::endl ;
	 else {
	   result = DNSServiceProcessResult(subref) ;
	   if (result!=kDNSServiceErr_NoError)
		std::cerr << "DNSServiceProcessResult failed" << std::endl ;   
	   DNSServiceRefDeallocate(subref) ;
	 }
    } else {
	 e->event = Event::LOST ;
    }

    browser->queue.push(e) ;
    browser->notifyObservers() ;
  }
#endif

  void
  DNSServiceBrowser::react(Observable *obs) {
#if NUCLEO_SUPPORTS_DNS_SD
    if (obs==watchdog && watchdog->getState()&FileKeeper::R) {
	 DNSServiceErrorType result = DNSServiceProcessResult(sdRef) ;
	 if (result!=kDNSServiceErr_NoError)
	   std::cerr << "DNSServiceProcessResult failed" << std::endl ;
    }
#endif
  }

  DNSServiceBrowser::Event *
  DNSServiceBrowser::getNextEvent(void) {
    if (queue.empty()) return 0 ;
    Event *e = queue.front() ;
    queue.pop() ;
    return e ;
  }

  void
  DNSServiceBrowser::Event::debug(std::ostream& out) const {
    out <<  service.interface << ": " 
	   << ((event==Event::FOUND) ? "found" : "lost") 
	   << " '" << service.name << "' (" 
	   << service.type << ", " << service.domain ;

    if (event==FOUND) {
	 out << ", " << service.host << ":" << service.port ;
	 for (DNSTextRecord::const_iterator pair=service.textRecord.begin();
		 pair!=service.textRecord.end();
		 ++pair) {
	   out << ", " << (*pair).first << "=" << (*pair).second ;
	 }
    }
  
    out << ")" << std::endl ;
  }

  DNSServiceBrowser::~DNSServiceBrowser(void) {
#if NUCLEO_SUPPORTS_DNS_SD
    unsubscribeFrom(watchdog) ;
    delete watchdog ;
    DNSServiceRefDeallocate(sdRef) ;
#endif
  }

}
