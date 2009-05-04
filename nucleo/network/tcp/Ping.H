#ifndef n_Ping_H
#define n_Ping_H

/*
 *
 * nucleo/network/tcp/Ping.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/URI.H>
#include <nucleo/core/TimeKeeper.H>
#include <nucleo/network/tcp/TcpConnection.H>

#include <stdexcept>
#include <iostream>

namespace nucleo {

  class Ping : public ReactiveObject {
    
  protected:

    typedef std::pair<std::string,int> hostport ;

    unsigned long milliseconds ;
    TimeKeeper *timer ;
    std::list<hostport> hosts ;
    unsigned int nbok ;

    void doCheck(void) ;
    void react(Observable* obs) ;
    
  public:
    
    Ping(int ms) ;
    double getState(void) ;
    double checkState(void) ;
    bool watch(std::string s) ;    
    ~Ping(void) ;
    
  } ;

}

#endif
