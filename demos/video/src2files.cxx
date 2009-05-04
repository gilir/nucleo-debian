/*
 *
 * demos/video/src2files.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/AppUtils.H>
#include <nucleo/core/ReactiveEngine.H>
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
    char *SOURCE = "videoin:" ;
    char *FORMAT = "img-%05d-%s.jpg" ;
    char *ENCODING = "JPEG" ;
    bool FLIP = false ;
    if (parseCommandLine(argc, argv, "i:f:e:F", "sssb", &SOURCE, &FORMAT, &ENCODING, &FLIP)<0) {
	 std::cerr << std::endl << "Usage: " << basename(argv[0])
			 << " [-i image-source] [-f filename-format] [-e image-encoding] [-F(lip)]"
			 << std::endl 
			 << "Example: " << basename(argv[0])
			 << " -i " << SOURCE << " -f " << FORMAT << " -e " << ENCODING << " -F"
			 << std::endl ;
	 exit(1) ;
    }

    Image img ;
    char filename[512] ;
    ImageFilter *filter = FLIP ? (ImageFilter *)new mirrorFilter('v') : 0 ;
    Image::Encoding encoding = Image::getEncodingByName(ENCODING) ;
    ImageSource *source = ImageSource::create(SOURCE) ;
    source->start() ;
    for (unsigned int i=0; source->getState()==ImageSource::STARTED;) {
	 ReactiveEngine::step() ;
	 if (source ->getNextImage(&img)) {
	   if (filter) filter->filter(&img) ;
	   convertImage(&img, encoding) ;
	   std::string timestamp = TimeStamp::createAsStringFromInt(img.getTimeStamp()) ;
	   snprintf(filename,512,FORMAT,i++,timestamp.c_str()) ;
	   std::cout << timestamp << " " << filename << std::endl ;
	   img.saveAs(filename) ;
	 }
    }
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }
  return 0 ;
}
