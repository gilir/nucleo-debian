/*
 *
 * nucleo/network/udp/StunResolver.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/udp/StunResolver.H>
#include <nucleo/network/udp/StunResolverPrivate.H>

#include <nucleo/core/FileKeeper.H>

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstring>

namespace nucleo {

  StunResolver::StunResolver(void) {
  }

  StunResolver::StunResolver(const char *server) {
    servers.push_back(server) ;
  }

  StunResolver::StunResolver(const char **serverss, unsigned int nbServers) {
    for (unsigned int i=0; i<nbServers; ++i)
	 servers.push_back(serverss[i]) ;
  }

  bool
  StunResolver::resolve(UdpSocket *socket,
				    std::string *host, int *port,
				    unsigned long timeout_in_ms) {
    if (socket->getProtocol()!=PF_INET) {
 	 std::cerr << "StunResolver::resolve: unsupported protocol, try IPv4" << std::endl ;
 	 return false ;
    }

    if (servers.empty()) {
 	 std::cerr << "StunResolver::resolve: empty server list" << std::endl ;
 	 return false ;
    }

    int verbose = 0 ;    
    sockaddr_storage server ;
    memset(&server,0,sizeof(sockaddr_storage)) ;
    for (std::list<std::string>::iterator s=servers.begin(); s!=servers.end(); ++s) {
	 if (socket->resolve((*s).c_str(), STUN_PORT, &server)) break ;
    }
    if (server.ss_family==PF_UNSPEC) return false ;
    if (verbose) std::cerr << "StunResolver::resolve: using server " << sockaddr2string(&server) << std::endl ;

    char buffer[STUN_MAX_MESSAGE_SIZE] ;
    StunMessage msg ; 
    memset(&msg, 0, sizeof(StunMessage)) ;
    StunAtrString username ; username.sizeValue = 0;
    StunAtrString password ; password.sizeValue = 0;
    stunBuildReqSimple(&msg, username, false, false, 1) ;
    int len = stunEncodeMessage(msg, buffer, STUN_MAX_MESSAGE_SIZE, password, verbose>1) ;
    ssize_t nbbytes = socket->send(buffer, len, &server) ;
    if (verbose)
	 std::cerr << "StunResolver::resolve: " << nbbytes << " bytes were sent to " << sockaddr2string(&server) << std::endl ;
    if (len!=nbbytes)
	 std::cerr << "StunResolver::resolve warning: only " << nbbytes << " bytes were sent, instead of " << len << std::endl ;

    FileKeeper *fk = FileKeeper::create(socket->getDescriptor(), FileKeeper::R) ;
    TimeKeeper *tk = TimeKeeper::create(timeout_in_ms) ;
    WatchDog rantanplan(tk) ;
    while (!rantanplan.sawSomething()) {
	 if (verbose)
	   std::cerr << "StunResolver::resolve: calling ReactiveEngine::step" << std::endl ;
	 ReactiveEngine::step(500*TimeKeeper::millisecond) ;
	 if (fk->getState()&FileKeeper::R) {
	   nbbytes = socket->receive(buffer, STUN_MAX_MESSAGE_SIZE) ;
	   if (verbose)
		std::cerr << "StunResolver::resolve: received " << nbbytes << " bytes" << std::endl ;
	   break ;
	 }
    }

    if (tk->getState()&TimeKeeper::TRIGGERED) {
	 if (verbose)
	   std::cerr << "StunResolver::resolve: no response from " << sockaddr2string(&server) << std::endl ;
	 return false ;
    }

    memset(&msg, 0, sizeof(StunMessage)) ;
    if (!stunParseMessage(buffer, nbbytes, msg, verbose>1)) {
	 if (verbose)
	   std::cerr << "StunResolver::resolve: error when parsing server response" << std::endl ;
	 return false ;
    }

    if (host) {
	 std::stringstream tmphost ;
	 uint32_t ip = msg.mappedAddress.ipv4.addr ;
	 tmphost << ((int)(ip>>24)&0xFF) << ".";
	 tmphost << ((int)(ip>>16)&0xFF) << ".";
	 tmphost << ((int)(ip>> 8)&0xFF) << ".";
	 tmphost << ((int)ip&0xFF) ;
	 *host = tmphost.str() ;
    }
    if (port) *port = msg.mappedAddress.ipv4.port ;
    return true ;
  }

  StunResolver::~StunResolver(void) {
  }

}
