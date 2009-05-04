/*
 *
 * tests/test-MemAlloc.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/TimeUtils.H>
#include <nucleo/image/Image.H>

#include <iostream>

using namespace nucleo ;

int
main(int argc, char **argv) {
  Chronometer chrono ;

  chrono.start() ;
  for (int i=0; i<1000; ++i) {
    Image img ;
    img.prepareFor(320,240,Image::ARGB) ;
    chrono.tick() ;
  }
  chrono.stop() ;
  std::cerr << chrono.read()/1000.0 << " sec" << std::endl ;
  std::cerr << chrono.average() << " images/sec" << std::endl ;

  std::cerr << std::endl ;

  Image img ;
  chrono.start() ;
  for (int i=0; i<1000; ++i) {
    img.prepareFor(320,240,Image::ARGB) ;
    chrono.tick() ;
  }
  chrono.stop() ;
  std::cerr << chrono.read()/1000000.0 << " sec" << std::endl ;
  std::cerr << chrono.average() << " images/sec" << std::endl ;
}
