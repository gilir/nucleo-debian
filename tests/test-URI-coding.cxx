/*
 *
 * tests/test-URI-coding.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/URI.H>

#include <iostream>
#include <string>

using namespace nucleo ;

int
main(int argc, char **argv) {
  if (argc!=3) {
    std::cerr << "Usage: " << argv[0] << " encode|decode string " << std::endl ;
    return -1 ;
  }

  std::string method(argv[1]) ;
  std::string input(argv[2]) ;

  if (method!="decode") {
    std::string output = URI::encode(input) ;
    std::cout << output << std::endl ;
  }

  if (method!="encode") {
    std::string output = URI::decode(input) ;
    std::cout << output << std::endl ;
  }

  return 0 ;
}
