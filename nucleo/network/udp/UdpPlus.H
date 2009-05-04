#ifndef n_UdpPlus_H
#define n_UdpPlus_H

/*
 *
 * nucleo/network/udp/UdpPlus.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <stdint.h>

namespace nucleo {

  namespace UdpPlus {

    struct Header {
	 uint16_t unum ;      // unit number
	 uint32_t totalsize ; // size of the whole unit
	 uint16_t fnum ;      // fragment number
    } ;

    extern const unsigned int HeaderSize ;
    extern const unsigned int FragmentSize ;

  }

}

#endif
