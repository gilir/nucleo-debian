/*
 *
 * tests/test-speed.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/AppUtils.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/image/sink/ImageSink.H>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

using namespace nucleo ;

int
main(int argc, char **argv) {
  try {

    char *SOURCE = 0 ;
    char *ENCODING = "PREFERRED" ;
    int NBIMAGES = 200 ;

    if (parseCommandLine(argc, argv, "i:n:e:", "sis", &SOURCE, &NBIMAGES, &ENCODING)<0) {
	 std::cerr << std::endl << argv[0] << " [-i source] {-e encoding-name] [-n nbimages]" << std::endl ;
	 exit(1) ;
    }

    ImageSource *source = ImageSource::create(SOURCE, Image::getEncodingByName(ENCODING)) ;

    std::cerr << "Getting " << NBIMAGES << " " << ENCODING << " images from " << SOURCE << std::endl ;

    source->start() ;
    for (int i=0; i<NBIMAGES; ++i) {
	 Image img ;
	 if (source->waitForImage(&img)) std::cerr << "+" << std::flush ;
	 else std::cerr << "-" << std::flush ;
    }
    
    std::cerr << std::endl << source->getMeanRate() << " frames/sec" << std::endl ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (...) {
    std::cerr << "Unknown exception..." << std::endl ;
  }
}
