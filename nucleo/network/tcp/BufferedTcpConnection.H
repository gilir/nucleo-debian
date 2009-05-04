#ifndef n_BufferedTcpConnection_H
#define n_BufferedTcpConnection_H

/*
 *
 * nucleo/network/tcp/BufferedTcpConnection.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/tcp/TcpConnection.H>

#include <cstring>
#include <queue>

namespace nucleo {

  class BufferedTcpConnection : public TcpConnection {

  private:

    struct buffer {
	 unsigned int offset ;
	 unsigned int size ;
	 char *data ;
	 buffer(char *d, unsigned int s) {
	   offset = 0 ;
	   size = s ;
	   data = d ;
	 }
	 ~buffer(void) {
	   delete [] data ;
	 }
    } ;

    std::queue<buffer*> buffers ;

    TcpConnection *connection ;
    unsigned int bufferSize ;

  protected:

    void react(Observable *obs) {
	 if (obs==_tcpw && (_tcpw->getState()&FileKeeper::R)) {
	   char *data = new char [bufferSize] ;
	   int size = TcpConnection::receive(data, bufferSize, false) ;
	   if (size>1) {
		buffers.push(new buffer(data, size)) ;
		if ((unsigned int)size==bufferSize) bufferSize = bufferSize*2 ;
		notifyObservers() ;
	   } else
		delete [] data ;
	 }
    }

  public:

    BufferedTcpConnection(std::string server_host, int server_port) 
	 : TcpConnection(server_host, server_port) {
	 bufferSize = 1024 ;
    }

    unsigned int receive(char *data, unsigned int length, bool complete) {
	 int needed = (int)length ;
	 unsigned int data_offset = 0 ;

	 while (! buffers.empty()) {
	   buffer *b = buffers.front() ;
	   int available = b->size - b->offset ;
	   if (available>(int)needed) {
		memmove(data+data_offset, b->data+b->offset, needed) ;
		b->offset += needed ;
		if (!_pendingNotifications) notifyObservers() ;
		return length ;
	   } else if (available==(int)needed) {
		memmove(data+data_offset, b->data+b->offset, needed) ;
		buffers.pop() ;
		delete b ;
		if (!buffers.empty() && !_pendingNotifications) notifyObservers() ;
		return length ;
	   } else {
		memmove(data+data_offset, b->data+b->offset, available) ;
		needed -= available ;
		data_offset += available ;
		buffers.pop() ;
		delete b ;
	   }
	 }

	 if (!complete) return length-needed ;

	 if (TcpConnection::receive(data+data_offset, needed, true))
	   return length ;

	 return 0 ;
    }

    ~BufferedTcpConnection(void) {
	 unsubscribeFrom(connection) ;
	 delete connection ;
    }

  } ;

}

#endif
