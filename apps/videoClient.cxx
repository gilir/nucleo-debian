/*
 *
 * apps/videoClient.cxx --
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
#include <nucleo/utils/SignalUtils.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/processing/convolution/Convolution.H>
#include <nucleo/image/processing/convolution/Blur.H>
#include <nucleo/image/processing/difference/Difference.H>
#include <nucleo/image/processing/basic/Resize.H>
#include <nucleo/image/processing/basic/Transform.H>
#include <nucleo/image/sink/ImageSink.H>
// #include <nucleo/gl/texture/glTexture.H>

#include <string>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

using namespace nucleo ;

class VideoClient : public ReactiveObject {

protected:

  ImageSource *source ;
  Image img ;
  ImageFilter *filter ;
  ImageSink *sink1, *sink2 ;

  int verbosity ;
  bool restart ;
  bool wait ;

  void react(Observable *obs) {
    if (sink1->getState()==ImageSink::STOPPED
	   && (!sink2 || sink2->getState()==ImageSink::STOPPED)) {
	 ReactiveEngine::stop() ;
	 return ;
    }

    if (source->getState()==ImageSource::STOPPED) {
	 if (restart) {
	   if (verbosity>1) std::cerr << "R" << std::flush ;
	   source->start() ;
	 } else if (!wait) {
	   ReactiveEngine::stop() ;
	   return ;
	 }
    }

    if (source->getNextImage(&img)) {
	 if (verbosity>10)
	   std::cerr << std::endl << img.getDescription() << std::endl ;
	 else if (verbosity>1)
	   std::cerr << "." << std::flush ;
	 if (filter) filter->filter(&img) ;
	 if (verbosity>10)
	   std::cerr << img.getDescription() << std::endl ;
	 sink1->handle(&img) ;
	 if (sink2) sink2->handle(&img) ;
    }
  }

  static void exitProc(int s) {
    ReactiveEngine::stop() ;
  }

public:

  VideoClient(int argc, char **argv) {
    verbosity = 0 ;
    restart = false ;
    wait = false ;

    char *SOURCE = 0 ;
    std::string FILTER = "None" ;
    char *SINK1=0, *SINK2=0 ;
    char *ENCODING = "PREFERRED" ;

    if (parseCommandLine(argc, argv, "v:e:i:f:o:p:wr", "issSssbb",
					&verbosity,
					&ENCODING,
					&SOURCE, &FILTER, &SINK1, &SINK2,
					&wait, &restart)<0) {
	 std::cerr << std::endl << argv[0] << " [-v verbosity] [-w(ait)] [-i source] [-e encoding] [-f filter] [-o sink1] [-p sink2] [-r(estart source when it stops)]" ;
	 std::cerr << std::endl ;
	 exit(1) ;
    }

    if (!SOURCE) SOURCE = getenv("NUCLEO_SRC1") ;
    if (!SOURCE) SOURCE = "videoin:" ;

    if (!SINK1) SINK1 = getenv("NUCLEO_DST1") ;
    if (!SINK1) SINK1 = "glwindow:" ;

    char *ENGINE = getenv("NUCLEO_ENGINE") ;
    ReactiveEngine::setEngineType(ENGINE?ENGINE:"default") ;

    Image::Encoding encoding = Image::getEncodingByName(ENCODING) ;
    if (verbosity) std::cerr << "encoding is " << ENCODING << std::endl ;
    source = ImageSource::create(SOURCE, encoding) ;
    subscribeTo(source) ;

    filter=0 ;
    if (FILTER=="edges")
	 filter = (ImageFilter *) new Convolution_3x3(0,-1,0, -1,4,-1,0, -1,0, 0,1) ;
    else if (FILTER=="horizontals")
	 filter = (ImageFilter *) new Convolution_3x3(-1,-1,-1, 0,0,0, 1,1,1, 1) ;
    else if (FILTER=="verticals")
	 filter = (ImageFilter *) new Convolution_3x3(-1,0,1, -1,0,1, -1,0,1, 1) ;
    else if (FILTER=="blur")
	 filter = (ImageFilter *) new BlurFilter(BlurFilter::HandV, 20, 1) ;
    else if (FILTER=="blur3")
	 filter = (ImageFilter *) new BlurFilter(BlurFilter::HandV, 20, 3) ;
    else if (FILTER=="boost")
	 filter = (ImageFilter *) new Convolution_3x3(-1,-1,-1, -1,21.6,-1, -1,-1,-1, 0,9) ;
    else if (FILTER=="emboss")
	 filter = (ImageFilter *) new Convolution_3x3(-1,-1,0, -1,0,1, 0,1,1, 120,1) ;
    else if (FILTER=="difference")
	 filter = (ImageFilter *) new DifferenceFilter ;
    else if (FILTER=="nobackground")
	 filter = (ImageFilter *) new DifferenceFilter(25,1) ;
    else if (!strncmp(FILTER.c_str(),"resize-",7))
	 filter = (ImageFilter *) new ResizeFilter(FILTER.c_str()+7) ;
    else if (!strncmp(FILTER.c_str(),"crop-",5))
	 filter = (ImageFilter *) new CropFilter(FILTER.c_str()+5) ;
    else if (FILTER=="flip-h")
	 filter = (ImageFilter *) new mirrorFilter('h') ;
    else if (FILTER=="flip-v")
	 filter = (ImageFilter *) new mirrorFilter('v') ;
    else if (FILTER=="rot+90")
	 filter = (ImageFilter *) new rotateFilter(true) ;
    else if (FILTER=="rot-90")
	 filter = (ImageFilter *) new rotateFilter(false) ;

    sink1 = ImageSink::create(SINK1) ;
    subscribeTo(sink1) ; 

    sink2=0 ;
    if (SINK2) {
	 sink2 = ImageSink::create(SINK2) ;
	 subscribeTo(sink2) ;
    }

    trapAllSignals(exitProc) ;
    source->start() ;
    sink1->start() ;
    if (SINK2) sink2->start() ;
    selfNotify() ;
  }

  ~VideoClient(void) {
    if (verbosity) {
	 std::cerr << std::endl ;
	 std::cerr << "source: " 
			 << source->getFrameCount() << " frame(s), "
			 << source->getMeanRate() << " fps" << std::endl ;
	 std::cerr << "sink1: "
			 << sink1->getFrameCount() << " frame(s), "
			 << sink1->getMeanRate() << " fps" << std::endl ;
	 if (sink2) std::cerr << "sink2: " << sink2->getMeanRate() << " fps" << std::endl ;
    }

    unsubscribeFrom(source) ;
    delete source ;
    unsubscribeFrom(sink1) ;
    delete sink1 ;
    if (sink2) {
	 unsubscribeFrom(sink2) ;
	 delete sink2 ;
    }
  }

} ;


int
main(int argc, char **argv) {
  try {
    // glTextureTile::debugLevel = 10 ;
    // glTexture::def_trePolicy = glTextureTile::FIRST_CHOICE ;
    // glTexture::def_generateMipmaps = false ;
    // glTexture::def_useClientStorage = true ;
    VideoClient client(argc, argv) ;
    ReactiveEngine::run() ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
