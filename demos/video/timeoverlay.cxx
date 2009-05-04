/*
 *
 * demos/timeoverlay.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/nucleo.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/image/processing/basic/Paint.H>
#include <nucleo/image/sink/ImageSink.H>

#include <iostream>
#include <stdexcept>

using namespace nucleo ;

// ---------------------------------------------------------------------------

int
main(int argc, char **argv) {
  try {
    char *SOURCE = getenv("NUCLEO_SRC1") ;
    char *DEST = getenv("NUCLEO_DST1") ;
    int STEP = 30 ;
    int HISTORY=5 ;
    float ALPHA = 0.5 ;

    if (parseCommandLine(argc, argv, "i:o:s:h:a:", "ssiif",
					&SOURCE, &DEST, &STEP, &HISTORY, &ALPHA)<0) {
	 std::cerr << std::endl << argv[0] ;
	 std::cerr << " [-i source] [-o dest] [-s step] [-h history] [-a alpha]" << std::endl ;
	 std::cerr << std::endl << std::endl ;
	 exit(1) ;
    }

    // ------------------------------------------------------

    ImageSource *source = ImageSource::create(SOURCE) ; // , Image::RGB) ;
    ImageSink *dest = ImageSink::create(DEST) ;

    source->start() ;
    dest->start() ;

    Image *imgs = new Image [HISTORY] ;
    int nbimages=0, nbimagesstored=0, current=0 ;
    Image newimage, composition ;

    for (bool loop=true; loop; ) {

	 if (source->getState()==ImageSource::STOPPED) source->start() ;
	 if (dest->getState()==ImageSink::STOPPED) break ;

	 ReactiveEngine::step() ;

	 if (source->getNextImage(&newimage)) {
	   if (nbimagesstored<HISTORY)
		composition.copyDataFrom(newimage) ;
	   else {
		int i ;
		composition.copyDataFrom(imgs[current]) ;
		// composition.debug(std::cerr) ; std::cerr << std::endl ;

		for (i=current+1; i<HISTORY; ++i) {
		  // imgs[i].debug(std::cerr) ; std::cerr << std::endl ;
		  blendImages(&composition, &imgs[i], &composition, ALPHA) ;
		}
		for (i=0; i<current; ++i) {
		  // imgs[i].debug(std::cerr) ; std::cerr << std::endl ;
		  blendImages(&composition, &imgs[i], &composition, ALPHA) ;
		}

		// newimage.debug(std::cerr) ; std::cerr << std::endl << std::endl ;
		blendImages(&composition, &newimage, &composition, ALPHA) ;
	   }

	   dest->handle(&composition) ;

	   if (! (nbimages%STEP)) {
		imgs[current].copyDataFrom(newimage) ;
		current = (current+1)%HISTORY ;
		nbimagesstored++ ;
	   }
	   nbimages++ ;
	 }

    }

    delete source ;
    delete [] imgs ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (...) {
    std::cerr << "Unknown error" << std::endl ;
  }

}
