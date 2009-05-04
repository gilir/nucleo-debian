#ifndef n_MD5_H
#define n_MD5_H

/*
 *
 * nucleo/utils/MD5.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <string>

#include <stdint.h>
#include <cstring>

namespace nucleo {

  class MD5 {

  public:

    typedef unsigned char bytearray[16] ;

  protected:

    uint32_t buf[4] ;
    uint32_t bits[2] ;
    unsigned char in[64] ;

    void transform(uint32_t *buf, uint32_t *in) ;

  public:

    MD5(void) ;

    void clear(void) ;

    void eat(std::string s) { eat((const unsigned char *)s.data(), s.size()) ; }
    void eat(const char *s) { eat((const unsigned char *)s, strlen(s)) ; }
    void eat(const unsigned char *data, unsigned int size) ;

    void digest(bytearray out) ;
    std::string digest(void) ;

  } ;

}

#endif
