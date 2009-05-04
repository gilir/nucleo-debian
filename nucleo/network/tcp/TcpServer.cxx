/*
 *
 * nucleo/network/tcp/TcpServer.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/network/tcp/TcpServer.H>
#include <nucleo/network/tcp/TcpUtils.H>

#include <nucleo/core/ReactiveEngine.H>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>

#include <stdexcept>
#include <iostream>

namespace nucleo {

  TcpServer::TcpServer(int port, int backlog, bool autoclose_connections) {
    _autoclose_connections = autoclose_connections ;
  
    _fd = socket(AF_INET, SOCK_STREAM, 0) ;
    if (_fd == -1)
	 throw std::runtime_error("TcpServer: can't create socket") ;

    setDefaultTcpSocketOptions(_fd, true) ;
  
    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(port);

    if (bind(_fd, (struct sockaddr *) &name, sizeof name) < 0)
	 throw std::runtime_error("TcpServer: bind failed") ;

    sockaddr_in myaddr;
    socklen_t lenmyaddr = sizeof(sockaddr_in);
    if (getsockname(_fd, (sockaddr *)&myaddr, &lenmyaddr)!=-1) {
	 _port = ntohs(myaddr.sin_port) ;
    } else {
	 _port = port ;
    }

    if (listen(_fd, backlog)==-1)
	 throw std::runtime_error("TcpServer: listen failed") ;

    _fk = FileKeeper::create(_fd, FileKeeper::R) ;
    // std::cerr << "TcpServer " << this << " uses FileKeeper " << _fk << std::endl ;
    subscribeTo(_fk) ;
  }

  void
  TcpServer::react(Observable*) {
    if (_fk->getState()&FileKeeper::R) {
	 int fd = accept(_fd,0,0) ;
	 if (fd!=-1)
	   _clientsWaiting.push(new TcpConnection(fd, _autoclose_connections)) ;
	 notifyObservers() ;
    }
  }

  std::string
  TcpServer::getHostName(void) {
    char buffer[256] ;
    gethostname(buffer,256) ;
    return buffer ;
  }

  TcpConnection *
  TcpServer::waitForNewClient(void) {
    while (_clientsWaiting.empty()) ReactiveEngine::step() ;
    return getNewClient() ;
  }

  TcpConnection *
  TcpServer::getNewClient(void) {
    if (_clientsWaiting.empty()) return 0 ;
    TcpConnection *client = _clientsWaiting.front() ;
    _clientsWaiting.pop() ;
    return client ;
  }

  TcpServer::~TcpServer(void) {
    unsubscribeFrom(_fk) ;
    delete _fk ;
    shutdown(_fd,2) ;
    close(_fd) ;
  }

}
