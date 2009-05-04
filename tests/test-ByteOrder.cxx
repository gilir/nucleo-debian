/*
 *
 * tests/test-ByteOrder.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/ByteOrder.H>

#include <iostream>

using namespace nucleo ;

int
main(int argc, char **argv) {
  std::cerr << "isLittleEndian: " << ByteOrder::isLittleEndian() << std::endl ;
  uint16_t test_16 = 0xf0ff ;
  uint32_t test_32 = 0xf0fff0ffL ;
  uint64_t test_64 = 0xf0fff0fff0fff0ffLL ;
  std::cerr << "16: " << test_16 << " " << ByteOrder::swap16ifle(test_16) << std::endl ;
  std::cerr << "32: " << test_32 << " " << ByteOrder::swap32ifle(test_32) << std::endl ;
  std::cerr << "64: " << test_64 << " " << ByteOrder::swap64ifle(test_64) << std::endl ;

  return 0 ;
}
