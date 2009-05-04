/*
 *
 * demos/misc/documentOpener.cxx --
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
#include <nucleo/image/sink/ImageSink.H>

#include <iostream>
#include <stdexcept>

using namespace nucleo ;

// ----------------------------------------------------------------------------

class Display : public ReactiveObject {

protected:

  std::string source ;
  ImageSource *src ;
  Image img ;
  ImageSink *dst ;

  void react(Observable *obs) {
    if (obs==src && src->getNextImage(&img))
	 dst->handle(&img) ;

    if (dst->getState()==ImageSink::STOPPED) delete this ;
  }

public:

  Display(std::string SOURCE) {
    source = SOURCE ;
    dst = ImageSink::create("glwindow:") ;  
    subscribeTo(dst) ;
    src = ImageSource::create(SOURCE) ;
    subscribeTo(src) ;
    dst->start() ;
    src->start() ;
  }

  ~Display(void) {
    unsubscribeFrom(src) ;
    delete src ;
    unsubscribeFrom(dst) ;
    delete dst ;
  }

} ;

// ----------------------------------------------------------------------------

class myOpener : public DocumentOpener {

protected:

  void openDocuments(std::vector<std::string> &documents) {
    std::cerr << "Open" << std::endl ;
    for (std::vector<std::string>::const_iterator i=documents.begin();
	    i!=documents.end();
	    ++i) {
	 std::cerr << "  " << *i << std::endl ;
	 new Display(*i) ;
    }
  }

} ;

int
main(int argc, char **argv) {
  try {
    myOpener opener ;
    ReactiveEngine::run() ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }
}
