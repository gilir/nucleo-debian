#ifndef n_TcpConnection_H
#define n_TcpConnection_H

/*
 *
 * nucleo/network/tcp/TcpConnection.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveObject.H>
#include <nucleo/core/FileKeeper.H>

#include <string>
#include <arpa/inet.h>

namespace nucleo {

  class TcpConnection : public ReactiveObject {

  protected:

    int _socket ;
    bool _closeSocketOnDestroy ;

    FileKeeper *_tcpw ;

    void connectTo(in_addr_t address, int port) ;
    
    void react(Observable *obs) {
	 if (obs==_tcpw && (_tcpw->getState()&FileKeeper::R))
	   notifyObservers() ;
    }

  public:

    TcpConnection(int socket, bool close=false) ;
    TcpConnection(in_addr_t address, int port) ; 
    TcpConnection(const char *hostname, int port) ; 
    TcpConnection(std::string hostname, int port) ; 

    int getFd(void) { return _socket ; }
    FileKeeper::eventMask getState(void) { return _tcpw->getState() ; }

    virtual unsigned int send(const char *data, unsigned int length, bool complete=true) ;
    virtual unsigned int receive(char *data, unsigned int length, bool complete) ;

    std::string machineLookUp(int *port=0) ;
    std::string userLookUp(int millisecs=500) ;

    ~TcpConnection(void) ;

  } ;

}

#endif
