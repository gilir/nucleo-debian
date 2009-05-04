#ifndef n_ByteOrder_H
#define n_ByteOrder_H

/*
 *
 * nucleo/utils/ByteOrder.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <stdint.h>

namespace nucleo {

  namespace ByteOrder {

    // Intel processors are little-endian, PowerPC and IP-based
    // network protocols are big-endian

    bool isLittleEndian(void) ;

    uint16_t swap16ifle(uint16_t arg) ;
    uint32_t swap32ifle(uint32_t arg) ;
    uint64_t swap64ifle(uint64_t arg) ;

  }

}

#endif
