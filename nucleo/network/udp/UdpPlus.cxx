/*
 *
 * nucleo/network/udp/UdpPlus.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/udp/UdpPlus.H>

namespace nucleo {

  // Max IP datagram size is 65535 bytes. IP header is 20 bytes and
  // UDP header is 8 bytes. That leaves us with at most 65507
  // bytes. However, the UDP/IP stack might support less than
  // that. 516 seems a minimum though...

  // static unsigned int maxUdpFragmentSize = 65535 ;
  // static unsigned int maxUdpFragmentSize = 32768 ;
  // static unsigned int maxUdpFragmentSize = 16384 ;
  static unsigned int maxUdpFragmentSize = 16300 ;
  // static unsigned int maxUdpFragmentSize = 16000 ;
  // static unsigned int maxUdpFragmentSize = 8192 ;
  
  const unsigned int UdpPlus::HeaderSize = 20+8 ;

  const unsigned int UdpPlus::FragmentSize =
    maxUdpFragmentSize
    - UdpPlus::HeaderSize
    - sizeof(UdpPlus::Header) ;

}
