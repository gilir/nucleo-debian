/*
 *
 * tests/test-MD5.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/MD5.H>

#include <iostream>

using namespace nucleo ;

int
main(int argc, char **argv) {
  MD5 md5 ;
  int totalbytes = 0 ;

  while (true) {
    unsigned char buffer[4096] ;
    int nbbytes = read(0, buffer, 4095) ;
    if (nbbytes<1) break ;
    md5.eat(buffer, nbbytes) ;
    totalbytes += nbbytes ;
  }

  std::cout << totalbytes << " bytes, MD5 = " << md5.digest() << std::endl ;

  return 0 ;
}
