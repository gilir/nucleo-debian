/*
 *
 * nucleo/network/dnssd/DNSServiceAnnouncer.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/dnssd/DNSServiceAnnouncer.H>

namespace nucleo {

#if NUCLEO_SUPPORTS_DNS_SD
  void
  DNSServiceAnnouncer::registerCallback(DNSServiceRef sdRef,
								DNSServiceFlags flags,
								DNSServiceErrorType errorCode,
								const char *name,
								const char *regtype,
								const char *domain,
								void *context) {
    DNSServiceAnnouncer *obj = (DNSServiceAnnouncer*)context ;
    if (errorCode==kDNSServiceErr_NoError) {
	 obj->name = name ;
	 obj->status = NOERROR ;
    } else if (errorCode==kDNSServiceErr_NameConflict) {
	 obj->status = NAMECONFLICT ;
    } else {
	 obj->status = ERROR ;
    }
  }
#endif

  DNSServiceAnnouncer::DNSServiceAnnouncer(void) {
#if NUCLEO_SUPPORTS_DNS_SD
    sdRef = 0 ;
#endif
    status = ERROR ;
  }

  DNSServiceAnnouncer::DNSServiceAnnouncer(const char *n, const char *t, uint16_t p,
								   DNSTextRecord *tr, 
								   const char *h, const char *d) {
    status = ERROR ;
#if NUCLEO_SUPPORTS_DNS_SD
    sdRef = 0 ;
    announce(n,t,p,tr,h,d) ;
#endif
  }

  bool
  DNSServiceAnnouncer::announce(const char *n, const char *t, uint16_t p,
						  DNSTextRecord *tr, 
						  const char *h, const char *d) {
#if NUCLEO_SUPPORTS_DNS_SD
    if (sdRef) { DNSServiceRefDeallocate(sdRef) ; sdRef = 0 ; }

    status = ERROR ;
    name = n?n:"" ; regtype = t?t:"" ; port = p ; domain = d?d:"" ; host = h?h:"" ; 
    textRecords = "" ;
    if (tr) {
	 std::stringstream formatted ;
	 for (DNSTextRecord::iterator r=tr->begin(); r!=tr->end(); ++r) {
	   std::string k=(*r).first, v=(*r).second ;
	   formatted << (unsigned char)(k.size()+1+v.size()) << k << "=" << v ;
	 }
	 textRecords = formatted.str() ;
    }
  
    if (kDNSServiceErr_NoError!=DNSServiceRegister(&sdRef,
										 0, // flags
										 0, // all interfaces
										 n, t,
										 d, h, htons(p),
										 textRecords.size(), textRecords.c_str(),
										 registerCallback,
										 (void *)this))
	 throw std::runtime_error("DNSServiceRegister failed") ;
  
    if (kDNSServiceErr_NoError!=DNSServiceProcessResult(sdRef))
	 std::cerr << "DNSServiceProcessResult failed" << std::endl ;
  
    return status==NOERROR ;
#else
    return false ;
#endif
  }

  DNSServiceAnnouncer::~DNSServiceAnnouncer(void) {
#if NUCLEO_SUPPORTS_DNS_SD
    DNSServiceRefDeallocate(sdRef) ;
#endif
  }

}
