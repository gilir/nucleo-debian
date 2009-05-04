/*
 *
 * tests/test-UUID.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/UUID.H>

#include <iostream>

using namespace nucleo ;

int
main(int argc, char **argv) {
  UUID uuid ;

  std::string uuid_asstring = uuid.getAsString() ;
  std::cerr << uuid_asstring << std::endl ;

  UUID::bytearray array ;
  uuid.getAsBytes(array) ;
  std::cerr << UUID::createAsStringFromBytes(array) << std::endl ;

  UUID::createAsBytesFromString(uuid_asstring, array) ;
  std::cerr << UUID::createAsStringFromBytes(array) << std::endl ;

  return 0 ;
}
