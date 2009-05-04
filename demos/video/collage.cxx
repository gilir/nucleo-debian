/*
 *
 * demos/collage.cxx --
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
#include <nucleo/utils/FileUtils.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/processing/basic/Resize.H>
#include <nucleo/image/sink/ImageSink.H>

#include <stdexcept>
#include <cstdlib>

using namespace nucleo ;

int
main(int argc, char **argv) {
  try {
    int WIDTH=-1, HEIGHT=-1 ;
    char *OUTPUT = 0 ;
    int NBIMG = -1 ;
    char *DIRECTORY = 0 ;

    int firstArg = parseCommandLine(argc, argv, "w:h:n:o:d:", "iiiss",
							 &WIDTH,&HEIGHT,&NBIMG,&OUTPUT,&DIRECTORY) ;
    if (firstArg<0 || (!DIRECTORY && firstArg==argc)) {
	 std::cerr << std::endl << argv[0] << " [-w width] [-h height] [-n nbimg] [-o sink] [-d directory] src1 ... srcn" << std::endl ;
	 exit(1) ;
    }

    if (!OUTPUT) OUTPUT = getenv("NUCLEO_DST1") ;
   
    std::vector<std::string> sources ;

    if (DIRECTORY) listFiles(DIRECTORY, &sources) ;

    const int nbArgs = argc-firstArg ;
    for (int i=0; i<nbArgs; ++i) sources.push_back(argv[firstArg+i]) ;

    std::cerr << sources.size() << " sources" << std::endl ;

    ImageSink *sink = ImageSink::create(OUTPUT) ;
    sink->start() ;

    Image img ;   

    for (std::vector<std::string>::const_iterator i=sources.begin();
	    i!=sources.end();
	    ++i) {
	 ImageSource *source = ImageSource::create(*i) ;
	 source->start() ;

	 std::cerr << "[" << std::flush ;
	 for (int nbimages=NBIMG; nbimages; ) {
	   if (source->getNextImage(&img)) {
		nbimages-- ;
		if (WIDTH==-1) WIDTH = img.getWidth() ;
		if (HEIGHT==-1) HEIGHT = img.getHeight() ;
		resizeImage(&img,WIDTH,HEIGHT) ;
		std::cerr << (sink->handle(&img) ? "+" : "-") << std::flush ;
	   }

	   if (source->getState()==ImageSource::STOPPED) break ;
	   ReactiveEngine::step() ;
	 }
	 std::cerr << "]" << std::flush ;

	 delete source ;
	 if (sink->getState()==ImageSink::STOPPED) break ;
    }    

    std::cerr << std::endl ;
    delete sink ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (...) {
    std::cerr << "Unknown error" << std::endl ;
  }

  return 0 ;
}
