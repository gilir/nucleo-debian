/*
 *
 * nucleo/core/UUID.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/core/UUID.H>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#else
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <fcntl.h>
#endif

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

namespace nucleo {

  // ----------------------------------------------------------------------

#ifdef __APPLE__

  UUID::UUID(void) {
    CFUUIDRef cfUUID = CFUUIDCreate(kCFAllocatorDefault) ;
    CFUUIDBytes cfUUIDbytes = CFUUIDGetUUIDBytes(cfUUID) ;
    memmove(bytes, (const void *)&cfUUIDbytes, 16) ;
    CFRelease(cfUUID) ;
  }

#else

  /*
   * The following code is adapted from gen_uuid.c, distributed under
   * the terms of the GNU Library General Public License, Copyright
   * (C) 1996, 1997, 1998, 1999 Theodore Ts'o.
   *
   * gen_uuid.c is part of e2fsprogs (http://e2fsprogs.sourceforge.net)
   */

  /*
  struct uuid {
    uint32_t	time_low;
    uint16_t	time_mid;
    uint16_t	time_hi_and_version;
    uint16_t	clock_seq;
    uint8_t	node[6]; 
  };
  */

  UUID::UUID(void) {   
    struct timeval tv ;
    gettimeofday(&tv, 0) ;

    int fd = open("/dev/urandom", O_RDONLY) ;
    if (fd == -1)
	 fd = open("/dev/random", O_RDONLY | O_NONBLOCK) ;
    if (fd==-1)
	 throw std::runtime_error("UUID::UUID: sorry, no random generator available...") ;
  
    srand((getpid() << 16) ^ getuid() ^ tv.tv_sec ^ tv.tv_usec) ;

    // Crank the random number generator a few times
    gettimeofday(&tv, 0) ;
    for (int i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--) rand() ;

    int n = 16 ;
    int lose_counter = 0 ;
    unsigned char *cp = (unsigned char *) bytes ;
    while (n > 0) {
	 int i = read(fd, cp, n) ;
	 if (i <= 0) {
	   if (lose_counter++ > 16) break ;
	   continue ;
	 }
	 n -= i ;
	 cp += i ;
	 lose_counter = 0 ;
    }
    
    // We do this all the time, but this is the only source of
    // randomness if /dev/random/urandom is out to lunch.
    for (unsigned int i = 0; i<16; ++i)
	 bytes[i] ^= (rand() >> 7) & 0xFF ;

    // clock_seq is bytes #8 and #9
    uint16_t clock_seq = (uint16_t(bytes[8]) << 8) + bytes[9] ;
    clock_seq = (clock_seq & 0x3FFF) | 0x8000 ;
    bytes[8] = clock_seq >> 8 ;
    bytes[9] = clock_seq & 0xFF ;

    // time_hi_and_version is bytes #6 and #7
    uint16_t time_hi_and_version = (uint16_t(bytes[6]) << 8) + bytes[7] ;
    time_hi_and_version = (time_hi_and_version & 0x0FFF) | 0x4000 ;
    bytes[6] = time_hi_and_version >> 8 ;
    bytes[7] = time_hi_and_version & 0xFF ;
  }
  
#endif

  void
  UUID::getAsBytes(bytearray output) {
    memmove(output, bytes, 16) ;
  }

  std::string
  UUID::getAsString(void) {
    return createAsStringFromBytes(bytes) ;
  }

  void
  UUID::createAsBytes(bytearray output) {
    UUID uuid ;
    uuid.getAsBytes(output) ;
  }

  void
  UUID::createAsBytesFromString(std::string input, bytearray output) {
    if (input.size()!=36) 
	 throw std::runtime_error("UUID::createAsBytesFromString bad input string") ;
    const char *ptr = input.data() ;
    for (unsigned int i=0, o=0; o<16; ++o) {
	 char hi = ptr[i++] ;
	 char low = ptr[i++] ;
	 output[o] = ((hi>64?(hi-55):(hi-48)) << 4) + (low>64?(low-55):(low-48)) ;
	 if (i==8 || i==13 || i==18 || i==23) ++i ;
    }
  }

  std::string
  UUID::createAsString(void) {
    UUID uuid ;
    return uuid.getAsString() ;
  }

  std::string
  UUID::createAsStringFromBytes(bytearray input) {
    char result[37] ;
    for (unsigned int i=0,j=0; i<36;) {
	 unsigned char b = input[j++] ;
	 unsigned char hi = b >> 4 ;
	 unsigned char lo = b & 0xF ;
	 result[i++] = (hi>9) ? (65+hi-10) : 48+hi ;
	 result[i++] = (lo>9) ? (65+lo-10) : 48+lo ;
	 if (i==8 || i==13 || i==18 || i==23) result[i++] = '-' ;
    }
    result[36] = '\0' ;
    return result ;
  }

  // ----------------------------------------------------------------------

}
