/*
 *
 * tests/test-Base64.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/AppUtils.H>
#include <nucleo/utils/Base64.H>

#include <iostream>
#include <cstdlib>

using namespace nucleo ;

int
main(int argc, char **argv) {
  bool ENCODE = false ;
  bool DECODE = false ;
  if (parseCommandLine(argc, argv, "ed", "bb", &ENCODE, &DECODE)<0) {
    std::cerr << std::endl << argv[0] ;
    std::cerr << " [-e(ncode)] [-d(ecode)]" << std::endl ;
    std::cerr << std::endl << std::endl ;
    exit(-1) ;
  }
  
  std::string input ;
  while (true) {
    char buffer[1024] ;
    int nbbytes = read(0, buffer, 1023) ;
    if (nbbytes<1) break ;
    input = input + std::string(buffer, nbbytes) ;
  }

  std::cerr << input.size() << " bytes" ;
  if (ENCODE) {
    input = Base64::encode(input) ;
    std::cerr << " --> " << input.size() << " bytes" ;
  }
  if (DECODE) {
    input = Base64::decode(input) ;
    std::cerr << " --> " << input.size() << " bytes" ;
  }
  std::cerr << std::endl ;

  std::cout << input ;

  return 0 ;
}
