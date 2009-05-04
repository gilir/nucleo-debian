#ifndef n_TcpServer_H
#define n_TcpServer_H

/*
 *
 * nucleo/network/tcp/TcpServer.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveObject.H>
#include <nucleo/core/FileKeeper.H>

#include <nucleo/network/tcp/TcpConnection.H>

#include <queue>

namespace nucleo {

  class TcpServer : public ReactiveObject {

  protected:

    int _fd, _port ;

    FileKeeper *_fk ;

    bool _autoclose_connections ;
    std::queue<TcpConnection*> _clientsWaiting ;

    void react(Observable*) ;

  public :

    // http://www.iana.org/assignments/port-numbers might help at this point...
    TcpServer(int port=0, int backlog=10, bool autoclose_connections=true) ;
    ~TcpServer(void) ;

    std::string getHostName(void) ;
    int getPortNumber(void) { return _port ; }

    // Observers are notified whenever one or more clients are
    // available
    TcpConnection *getNewClient(void) ;

    TcpConnection *waitForNewClient(void) ;
  } ;

}

#endif
