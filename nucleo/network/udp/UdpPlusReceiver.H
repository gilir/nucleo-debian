#ifndef n_UdpPlusReceiver_H
#define n_UdpPlusReceiver_H

/*
 *
 * nucleo/network/udp/UdpPlusReceiver.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/udp/UdpPlus.H>

#include <nucleo/core/ReactiveObject.H>
#include <nucleo/core/FileKeeper.H>

namespace nucleo {

  // ------------------------------------------------------

  class UdpPlusReceiver : public ReactiveObject {

  private:

    int _port ;
    int _socket ;

    unsigned char *_buffer ;
    unsigned int _bufferSize ;

    enum {OUT_OF_SYNC, INCOMPLETE, READY} _state ;
    UdpPlus::Header _header ;
    uint16_t _unum, _fnum ;
    unsigned int _bytesRead ;
    bool _newUnitReady ;

    FileKeeper *_fk ;

    void _open(const int port, const char *mcastGroup=0) ;

  protected:

    void react(Observable *) ;

  public:

    UdpPlusReceiver(const int port=0) {
	 _open(port) ;
    }

    UdpPlusReceiver(const char *mcastGroup, const int port) {
	 _open(port, mcastGroup) ;
    }
  
    // It would be nice to be able to create a UdpPlusSender from a
    // message received by a UdpPlusReceiver...

    int getPortNumber(void) const { return _port ; }

    // If true, the first size bytes of data is the message. Data
    // should be deleted (with delete []) when it is not needed
    // anymore
    bool receive(unsigned char **data, unsigned int *size) ;
 
    ~UdpPlusReceiver(void) ;

  } ;

  // ------------------------------------------------------

}

#endif
