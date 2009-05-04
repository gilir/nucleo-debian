/*
 *
 * demos/video/multiplex.cxx --
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
#include <nucleo/image/processing/basic/Paint.H>
#include <nucleo/image/processing/basic/Resize.H>
#include <nucleo/image/sink/ImageSink.H>
#include <nucleo/gl/window/glWindow.H>

#include <stdexcept>
#include <vector>

using namespace nucleo ;

// ---------------------------------------------------------------

class Multiplex : public ReactiveObject {

  std::vector<ImageSource *> sources ;
  Image composite ;
  ImageSink *sink ;
  int width, height, subwidth, subheight ;
  unsigned int sq ;
  bool autorestart ;

protected:

  static void exitProc(int) {
    ReactiveEngine::stop() ;
  }

  void react(Observable *obs) {
    if (sink->getState()==ImageSink::STOPPED) ReactiveEngine::stop() ;

    bool newComposite = false ;

    for (unsigned int i=0; i<sources.size(); ++i) {
	 ImageSource *source = sources[i] ;
	  
	 if (autorestart && source->getState()==ImageSource::STOPPED) source->start() ;

	 Image img ;
	 if (source->getNextImage(&img)) {
	   resizeImage(&img,subwidth,subheight) ;
	   int x = (i%sq)*subwidth ;
	   int y = (i/sq)*subheight ;
	   drawImageInImage(&img, &composite, x, y) ;
	   newComposite = true ;
	 }

    }

    if (newComposite) sink->handle(&composite) ;
  }
  
public:

  Multiplex(char *uri, int w, int h, bool ar) {
    sink = ImageSink::create(uri) ;
    subscribeTo(sink) ;
    sink->start() ;

    width = w ; height = h ;
    autorestart = ar ;
    subwidth = subheight = 0 ;

    composite.prepareFor(width,height,Image::RGB) ;
    paintImage(&composite,0,0,60,255) ;

    trapAllSignals(exitProc) ;
  }

  void addSource(char *uri) {
    try {
	 ImageSource *source = ImageSource::create(uri) ;
	 subscribeTo(source) ;
	 source->start(); 
	 sources.push_back(source) ;
	 
	 sq=0 ; while (sq*sq<sources.size()) ++sq ;
	 subwidth=width/sq ;
	 subheight=height/sq ;
    } catch (std::runtime_error err) {
	 std::cerr << "Runtime error: " << err.what() << std::endl ;
    }
  }

  ~Multiplex(void) {
    while (!sources.empty()) {
	 ImageSource *source = sources.back() ;
	 sources.pop_back() ;
	 unsubscribeFrom(source) ;
	 delete source ;
    }

    unsubscribeFrom(sink) ;
    delete sink ;
  }

} ;

int
main(int argc, char **argv) {
  try {
    int WIDTH=160, HEIGHT=120 ;
    char *OUTPUT = "glwindow:" ;
    bool AUTORESTART = false ;

    int firstArg = parseCommandLine(argc, argv, "w:h:o:r", "iisb", &WIDTH,&HEIGHT,
							 &OUTPUT,&AUTORESTART) ;
    if (firstArg<0 || firstArg==argc) {
	 std::cerr << std::endl << argv[0] << " [-r] [-w width] [-h height] [-o sink] src1 ... srcn" << std::endl ;
	 exit(1) ;
    }

    Multiplex multiplex(OUTPUT, WIDTH, HEIGHT, AUTORESTART) ;

    for (int i=0; i<argc-firstArg; ++i)
	 multiplex.addSource(argv[firstArg+i]) ;

    ReactiveEngine::run() ;
    
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (...) {
    std::cerr << "Unknown error" << std::endl ;
  }

}
