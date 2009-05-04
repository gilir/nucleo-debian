/*
 *
 * demos/video/blender.cxx --
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
#include <nucleo/image/processing/chromakeying/ChromaKeyingFilter.H>
#include <nucleo/image/sink/ImageSink.H>
#include <nucleo/gl/window/glWindow.H>

#include <stdexcept>

using namespace nucleo ;

// ---------------------------------------------------------------

class Blender : public ReactiveObject {

  ImageSource *source1, *source2 ;
  Image image1, image2, composite ;
  bool useImage1, useImage2 ;
  ChromaKeyingFilter *keyer ;
  ImageSink *dest ;
  bool chromakeying ;
  float alpha ;

protected:

  static void exitProc(int) {
    ReactiveEngine::stop() ;
  }

  void react(Observable *obs) {
    if (dest->getState()==ImageSink::STOPPED) {
	 ReactiveEngine::stop() ;
	 return ;
    }

    if (source1->getState()==ImageSource::STOPPED) source1->start() ;
    if (source2->getState()==ImageSource::STOPPED) source2->start() ;
    
    if (source1->getNextImage(&image1)) useImage1 = true ;
    else if (source1->getState()==ImageSource::STOPPED) useImage1 = false ;

    if (source2->getNextImage(&image2)) {
	 if (keyer) keyer->filter(&image2) ;
	 useImage2 = true ;
    } else {
	 if (source2->getState()==ImageSource::STOPPED) useImage2 = false ;
    }

    if (useImage1 && useImage2) {
	 // image1.debug(std::cerr) ; std::cerr << " " ; image2.debug(std::cerr) ; std::cerr << std::endl ;
	 // std::cerr << "b" << std::flush ;
	 resizeImage(&image2, image1.getWidth(), image1.getHeight()) ;
	 if (chromakeying)
	   blendImages(&image1, &image2, &composite) ;
	 else
	   blendImages(&image1, &image2, &composite, alpha) ;
    } else {
	 if (useImage2) {
	   std::cerr << "2" << std::flush ;
	   composite.copyDataFrom(image2) ;
	 } else if (useImage1) {
	   std::cerr << "1" << std::flush ;
	   composite.copyDataFrom(image1) ;
	 }
    }
    
    dest->handle(&composite) ;
  }
  
public:

  Blender(char *SRC1, char *SRC2, char *DST, float a, bool ck) {
    alpha = a ;
    chromakeying = ck ;
    if (chromakeying) {
	 std::cerr << "chromakeying mode" << std::endl ;
	 keyer = new ChromaKeyingFilter(71,86,112,23,96) ;
	 source1 = ImageSource::create(SRC1, Image::ARGB) ;
	 source2 = ImageSource::create(SRC2, Image::ARGB) ;
    } else {
	 std::cerr << "simple blend mode" << std::endl ;
	 keyer = 0 ;
	 source1 = ImageSource::create(SRC1) ;
	 source2 = ImageSource::create(SRC2) ;
    }

    subscribeTo(source1) ;
    source1->start() ;
    subscribeTo(source2) ;
    source2->start() ;

    dest = ImageSink::create(DST) ;
    subscribeTo(dest) ;
    dest->start() ;

    useImage1 = useImage2 = false ;

    trapAllSignals(exitProc) ;
  }

  ~Blender(void) {
    std::cerr << dest->getMeanRate() << " frames/second" << std::endl ;
    unsubscribeFrom(dest) ;
    delete dest ;

    delete keyer ;

    unsubscribeFrom(source1) ;
    delete source1 ;
    unsubscribeFrom(source2) ;
    delete source2 ;
  }

} ;

int
main(int argc, char **argv) {
  try {
    char *SRC1 = 0 ;
    char *SRC2 = 0 ;
    char *DST = 0 ;
    float ALPHA = 0.5 ;
    bool CHROMAKEYING = false ;

    if (parseCommandLine(argc, argv, "o:i:j:a:c", "sssfb",
					&DST, &SRC1, &SRC2, &ALPHA, &CHROMAKEYING)<0) {
	 std::cerr << std::endl << argv[0] ;
	 std::cerr << " [-i source1] [-j source2] [-a alpha] [-c] [-o dest]" << std::endl ;
	 std::cerr << "source2 is resized and overlaid on source1" << std::endl ;
	 std::cerr << "alpha=0.0 is source2 only, alpha=1.0 is source1 only" << std::endl ;
	 std::cerr << std::endl << std::endl ;
	 exit(1) ;
    }

    if (!SRC1) SRC1 = getenv("NUCLEO_SRC1") ;
    if (!SRC1) SRC1 = "videoin:/anydev/anynode" ;

    if (!SRC2) SRC2 = getenv("NUCLEO_SRC2") ;
    if (!SRC2) SRC2 = "videoin:/anydev/anynode" ;

    if (!DST) DST = getenv("NUCLEO_DST1") ;
    if (!DST) DST = "glwindow:" ;

    Blender blender(SRC1, SRC2, DST, ALPHA, CHROMAKEYING) ;

    ReactiveEngine::run() ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (...) {
    std::cerr << "Unknown error" << std::endl ;
  }
    
}
