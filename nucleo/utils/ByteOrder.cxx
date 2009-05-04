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

#include <nucleo/utils/ByteOrder.H>

#ifdef __APPLE__
#include <CoreFoundation/CFByteOrder.h>
#endif

namespace nucleo {
  namespace ByteOrder {
#ifdef __APPLE__
    bool isLittleEndian(void) {
	 return CFByteOrderGetCurrent()==CFByteOrderLittleEndian ;
    }
    uint16_t swap16ifle(uint16_t arg) {
	 return CFSwapInt16HostToBig(arg) ;
    }
    uint32_t swap32ifle(uint32_t arg) {
	 return CFSwapInt32HostToBig(arg) ;
    }
    uint64_t swap64ifle(uint64_t arg) {
	 return CFSwapInt64HostToBig(arg);
    }
#else
    bool isLittleEndian(void) {
	 static uint32_t littleEndianTest = 1 ;
	 return (*(char *)&littleEndianTest == 1) ;
    }
    uint16_t swap16ifle(uint16_t arg) {
	 if (isLittleEndian())
	   return ((((arg) & 0xff) << 8) | (((arg) >> 8) & 0xff)) ;
	 else
	   return arg ;
    }
    uint32_t swap32ifle(uint32_t arg) {
	 if (isLittleEndian())
	   return ((((arg) & 0xff000000) >> 24) | \
			 (((arg) & 0x00ff0000) >> 8)  | \
			 (((arg) & 0x0000ff00) << 8)  | \
			 (((arg) & 0x000000ff) << 24)) ;
	 else
	   return arg ;
    }
    uint64_t swap64ifle(uint64_t arg) {
	 if (isLittleEndian()) {
	   uint32_t *ptr = (uint32_t*)&arg ;
	   uint32_t tmp = swap32ifle(ptr[0]) ;
	   ptr[0] = swap32ifle(ptr[1]) ;
	   ptr[1] = tmp ;
	 }
	 return arg ;
    }

#endif
  }
}
