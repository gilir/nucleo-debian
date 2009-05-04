/*
 *
 * tests/test-StringUtils.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/StringUtils.H>

#include <iostream>
#include <string>

using namespace nucleo ;

int
main(int argc, char **argv) {
  cistring headers( "Content-Type: text/html\nContent-Length: 12" );
  const cistring key("content-length") ;
  std::cout << headers.find(key,0) << std::endl ;
  std::cout << headers.find("connection") << std::endl ;

  std::cout << "-------------------------------------------" << std::endl ;

  std::string word, request = "GET /test HTTP/1.0" ;

  do {
    word = extractNextWord(request) ;
    std::cout << "|" << word << "| (" << request << ")" << std::endl ;
  } while (word != "") ;

  return 0 ;
}
