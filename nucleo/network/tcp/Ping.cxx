/*
 *
 * nucleo/network/tcp/Ping.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/tcp/Ping.H>

namespace nucleo {

  void
  Ping::doCheck(void) {
    timer->disarm() ;

    nbok = 0 ;
    // std::cerr << "Ping: checking " << hosts.size() << " hosts" << std::endl ;
    for (std::list<hostport>::iterator hp=hosts.begin();
	    hp!=hosts.end(); ++hp) {
	 try {
	   TcpConnection *connection = new TcpConnection((*hp).first, (*hp).second) ;
	   // std::cerr << "   connected to " << (*hp).first << ":" << (*hp).second << std::endl ;		
	   delete connection ;
	   nbok++ ;
	 } catch (std::runtime_error e) {
	   // std::cerr << "   failed to connect to " << (*hp).first << ":" << (*hp).second << std::endl ;
	 }
    }
    // std::cerr << "Ping: " << 100.0*(double)nbok/(double)hosts.size() << "% available" << std::endl ;
    
    timer->arm(milliseconds*TimeKeeper::millisecond) ;
  }
    
  void
  Ping::react(Observable* obs) {
    if (obs!=timer || !hosts.size()) return ;
    unsigned int old = nbok ;
    doCheck() ;
    if (nbok!=old) notifyObservers() ;
  }

  Ping::Ping(int ms) {
    milliseconds = ms ;
    timer = TimeKeeper::create() ;
    subscribeTo(timer) ;
    timer->arm(milliseconds*TimeKeeper::millisecond) ;
  }
  
  double
  Ping::getState(void) {
    return (double)nbok/(double)hosts.size() ;
  }

  double
  Ping::checkState(void) {
    doCheck() ;
    return (double)nbok/(double)hosts.size() ;
  }
    
  bool
  Ping::watch(std::string s) {
    URI uri(s) ;
    if (uri.scheme=="http") {
	 if (!uri.port) uri.port = 80 ;
    } else if (uri.scheme!="tcp") {
	 // std::cerr << "Ping: unsupported scheme (" << uri.scheme << ")" << std::endl ;
	 return false ;
    }
    // std::cerr << "Ping: adding " << s << " (" << uri.host << ", " << uri.port << ")" << std::endl ;
    hosts.push_back(hostport(uri.host,uri.port)) ;
    return true ;
  }
    
  Ping::~Ping(void) {
    unsubscribeFrom(timer) ;
    delete timer ;
  }

}
