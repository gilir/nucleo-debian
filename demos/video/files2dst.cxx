/*
 *
 * demos/video/files2dst.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/AppUtils.H>
#include <nucleo/utils/FileUtils.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/image/sink/ImageSink.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/processing/basic/Transform.H>
#include <nucleo/image/encoding/Conversion.H>

#include <stdexcept>
#include <cstdlib>

#include <libgen.h>

using namespace nucleo ;

int
main(int argc, char **argv) {
  try {
    char *SOURCE = "." ;
    char *SINK = "test.mov" ;
    char *ENCODING = "JPEG" ;
    bool FLIP = false ;
    if (parseCommandLine(argc, argv, "i:o:e:F", "ssssb", 
					&SOURCE, &SINK, &ENCODING, &FLIP)<0) {
	 std::cerr << std::endl << "Usage: " << basename(argv[0])
			 << " [-i source-directory] [-o image-destination] [-e image-encoding] [-F(lip)]"
			 << std::endl 
			 << "Example: " << basename(argv[0])
			 << " -i " << SOURCE << " -o " << SINK << " -e " << ENCODING
			 << std::endl ;
	 exit(1) ;
    }

    std::vector<std::string> filenames ;
    if (!listFiles(SOURCE, &filenames)) return 1 ;

    Image img ;
    ImageFilter *filter = FLIP ? (ImageFilter *)new mirrorFilter('v') : 0 ;
    Image::Encoding encoding = Image::getEncodingByName(ENCODING) ;
    ImageSink *sink = ImageSink::create(SINK) ;
    sink->start() ;
    for (std::vector<std::string>::iterator i=filenames.begin();
	    sink->getState()==ImageSink::STARTED && i!=filenames.end(); ++i) {
	 ReactiveEngine::step(5) ;
	 try {
	   ImageSource::getImage(*i, &img, encoding) ;
	   std::cout << *i << " --> " << img.getDescription() << std::endl ;
	   if (img.getWidth() && img.getHeight()) {
		if (filter) filter->filter(&img) ;
		sink->handle(&img) ;
	   }
	 } catch (...) {
	 }
    }
    sink->stop() ;
    delete sink ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }
  return 0 ;
}
