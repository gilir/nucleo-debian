/*
 *
 * demos/video/mini-mirrorSpace.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/nucleo.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/processing/convolution/Blur.H>
#include <nucleo/gl/window/glWindow.H>

#include <stdexcept>
#include <cstdlib>

using namespace nucleo ;

class Reactor : public ReactiveObject {

protected:

  Image image ;
  BlurFilter filter ;
  int size, repeat ;
  ImageSource *source ;
  glWindow *window ;

  void react(Observable *obs) {
    bool debug=false, redisplay=false ;

    if (obs==window) {
	 glWindow::event e ;
	 while (window->getNextEvent(&e)) {
	   switch (e.type) {
	   case glWindow::event::expose:
		redisplay = true ;
		break ;
	   case glWindow::event::keyRelease:
		switch (e.keysym) {
		case XK_Escape: ReactiveEngine::stop() ; break ;
		case XK_Left: if (size>0) size--; debug=true ; break ;
		case XK_Right: size++ ; debug=true ; break ;
		case XK_Up: repeat++ ; debug=true ; break ;
		case XK_Down: if (repeat>0) repeat--; debug=true ; break ;
		}
		break ;
	   default: break ;
	   }
	 }
    }

    if (obs==source) {
	 if (source->getNextImage(&image)) {
	   if (source->getFrameCount()==1) {
		unsigned int width=image.getWidth(), height=image.getHeight() ;
		window->setGeometry(width, height, 500, 120) ;
		glViewport(0,0,width,height) ;
	   }
	   filter.filter(&image, BlurFilter::HandV, size, repeat) ;
	   redisplay = true ;
	 }
    }

    if (debug) std::cerr << "s=" << size << " r=" << repeat << std::endl ;

    if (redisplay) {
	 glClear(GL_COLOR_BUFFER_BIT) ;
	 glRasterPos2f(-0.5, 0.5) ;
	 glDrawPixels(image.getWidth(), image.getHeight(),
			    GL_RGB, GL_UNSIGNED_BYTE, image.getData());
	 window->swapBuffers() ;
    }

  }
public:
  Reactor(ImageSource *s, glWindow *w) {
    source = s ;
    subscribeTo(source) ;
    window = w ;
    subscribeTo(window) ;   
    size = 3 ;
    repeat = 2 ;
  }
} ;

int
main(int argc, char **argv) {
  try {
    char *SOURCE = "videoin:/anydev/anynode" ;
    if (parseCommandLine(argc, argv, "i:", "s", &SOURCE)<0) {
	 std::cerr << std::endl << argv[0] << " [-i source]" ;
	 std::cerr << std::endl ;
	 exit(1) ;
    }

    long options = glWindow::DOUBLE_BUFFER ;
    long eventmask = glWindow::event::expose|glWindow::event::keyRelease ;
    glWindow *window = glWindow::create(options, eventmask) ;

    window->setTitle("mini-mirrorSpace") ;

    window->makeCurrent() ;
    glMatrixMode(GL_PROJECTION) ;
    glLoadIdentity() ;
    glOrtho(-0.5,0.5,-0.5,0.5,-1.0,1.0) ;
    glMatrixMode(GL_MODELVIEW) ;
    glPixelZoom(1,-1) ;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1) ;
    glClearColor(1,1,1,1) ;

    ImageSource *source = ImageSource::create(SOURCE, Image::RGB) ;
    source->start() ;

    Reactor tester(source, window) ;

    ReactiveEngine::run() ;

    delete source ;
    delete window ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }
}
