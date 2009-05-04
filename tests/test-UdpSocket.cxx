/*
 *
 * tests/test-UdpSocket.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/network/NetworkUtils.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/network/udp/UdpSocket.H>
#include <nucleo/network/udp/StunResolver.H>

#include <errno.h>

#include <stdexcept>
#include <iostream>
#include <cstdlib>

using namespace nucleo ;

// --------------------------------------------------------------------

class Tester : public ReactiveObject {

protected:

  UdpSocket *socket ;
  char *buffer ;
  unsigned int bufferSize ;

  void react(Observable *obs) {
    sockaddr_storage peer ;
#if 0
    int nbbytes = socket->receive(buffer, bufferSize, &peer) ;
#else
    struct iovec iov[1] ;
    iov[0].iov_base = buffer ;
    iov[0].iov_len = bufferSize ;
    int nbbytes = socket->receive(iov, 1, &peer) ;
#endif
    std::cerr << "Received " << nbbytes << " bytes from " << sockaddr2string(&peer) ;
    if (nbbytes<64) std::cerr << " (" << std::string(buffer,nbbytes) << ")" ;
    std::cerr << std::endl ;
  }

public:

  Tester(UdpSocket *s) {
    bufferSize = 1024*16 ;
    buffer = new char [bufferSize] ;
    socket = s ;
    subscribeTo(socket) ;
  }

  ~Tester(void) {
    delete [] buffer ;
    unsubscribeFrom(socket) ;
    delete socket ;
  }

} ;

// --------------------------------------------------------------------

int
main(int argc, char **argv) {
  bool IPv6 = false ;
  int PORT = 0 ;
  char *GROUP = 0 ;
  int firstArg = parseCommandLine(argc, argv, "6p:g:", "bis", &IPv6, &PORT, &GROUP) ;
  if (firstArg<0) {
    std::cerr << "Usage: " << argv[0] << " [-p port] [-g multicast-group]" << std::endl
		    << "  multicast group examples: FF01:0:0:0:0:0:0:AA, 225.0.0.252" << std::endl ;
    exit(-1) ;
  }

  UdpSocket socket(IPv6?PF_INET6:PF_INET) ;

  char *bomdia = "bomdia" ;
  struct iovec bomdia_iov = {bomdia, 7} ;

  std::cerr << "listening " ;
  if (GROUP) std::cerr << "multicast group " << GROUP ;
  std::cerr << "on " << (IPv6?"IPv6":"IPv4") << " port " << PORT 
		  << " -> " << socket.listenTo(PORT, GROUP) ;
  std::cerr << " (" << getHostName() << ":" << socket.getPortNumber() << ")"
		  << std::endl ;

  sockaddr_storage peer ;
  if (socket.resolve("localhost",8888,&peer)) {
    std::cerr << "sending 8 bytes to " <<  sockaddr2string(&peer) 
		    << " -> " << socket.send("hithere",8,&peer) << std::endl ;
  } else
    std::cerr << "resolve failed" << std::endl ;

  std::cerr << "sending a 7 bytes iovec to " << sockaddr2string(&peer) << " -> " << socket.send(&bomdia_iov, 1, &peer) << std::endl ;

  std::cerr << "sending 8 bytes -> " << socket.send("hithere",8) << std::endl ;

  std::cerr << "connecting to dummy:7777 -> " << socket.connectTo("dummy","7777") << std::endl ;
  socket.getPeerName(&peer) ;
  std::cerr << "connected to " << sockaddr2string(&peer) << std::endl ;

  std::cerr << "connecting to localhost:8888 -> " << socket.connectTo("localhost",8888) << std::endl ;
  socket.getPeerName(&peer) ;
  std::cerr << "connected to " << sockaddr2string(&peer) << std::endl ;

  std::cerr << "sending 8 bytes -> " << socket.send("bonjour",8) << std::endl ;

  std::cerr << "sending a 7 bytes iovec -> " << socket.send(&bomdia_iov, 1) << std::endl ;
  std::cerr << "sending a 7 bytes iovec to " << sockaddr2string(&peer) << " -> " << socket.send(&bomdia_iov, 1, &peer) << std::endl ;

  std::cerr << "disconnecting -> " << socket.disconnect() << std::endl ;

  std::cerr << "sending 7 bytes to " << sockaddr2string(&peer) << " -> " << socket.send("hohoho",7,&peer) << std::endl ;

  std::cerr << "sending a 7 bytes iovec to " << sockaddr2string(&peer) << " -> " << socket.send(&bomdia_iov, 1, &peer) << std::endl ;

  if (IPv6) {
    if (socket.resolve("FF01:0:0:0:0:0:0:AA",8888,&peer)) {
	 std::cerr << "sending 11 bytes to " << sockaddr2string(&peer)
			 << " -> " << socket.send("multicast6",11,&peer) << std::endl ;
    } else
	 std::cerr << "resolve failed" << std::endl ;
  } else {
    if (socket.resolve("225.0.0.252",8888,&peer)) {
	 std::cerr << "sending 11 bytes to " << sockaddr2string(&peer) 
			 << " -> " << socket.send("multicast4",11,&peer) << std::endl ;
    } else
	 std::cerr << "resolve failed" << std::endl ;
  }

  StunResolver stun ;
  std::string public_host = getHostName() ;
  int public_port = socket.getPortNumber() ;
  if (stun.resolve(&socket, &public_host, &public_port, 5*TimeKeeper::second)) 
    std::cerr << "STUN-mapped address: " << public_host << ":" << public_port << std::endl ;
  else
    std::cerr << "STUN mapping failed" << std::endl ;

  Tester tester(&socket) ;

  ReactiveEngine::run() ;

  return 0 ;
}
