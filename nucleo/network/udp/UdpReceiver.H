#ifndef n_UdpReceiver_H
#define n_UdpReceiver_H

/*
 *
 * nucleo/network/udp/UdpReceiver.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveObject.H>
#include <nucleo/core/FileKeeper.H>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace nucleo {

  class UdpReceiver : public ReactiveObject {

  private:

    int _socket ;
    int _port ;
    struct sockaddr_in _from ;  
    socklen_t _fromlen ;
    int _rcvbufsize ;

    FileKeeper *_fk ;

    void _open(const int port, const char *mcastGroup=0) ;

  protected:

    void react(Observable*) ;

  public:

    // http://www.iana.org/assignments/port-numbers might help at this point...
    UdpReceiver(const int port=0) {
	 _open(port) ;
    }

    UdpReceiver(std::string mcastGroup, const int port) {
	 _open(port, mcastGroup.c_str()) ;
    }

    UdpReceiver(const char *mcastGroup, const int port) {
	 _open(port, mcastGroup) ;
    }

    int getPortNumber(void) const { return _port ; }

    bool setBufferSize(int size) ;

    // If true, data is the message. Data should be deleted (with
    // delete []) when it is not needed anymore
    bool receive(unsigned char **data, unsigned int *size) ;

    ~UdpReceiver(void) ;

  } ;

}

#endif
