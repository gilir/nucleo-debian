/*
 *
 * nucleo/demos/vnc/vnc-relay.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/nucleo.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/plugins/vnc/vncImageSource.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/utils/FileUtils.H>
#include <nucleo/gl/glUtils.H>
#include <nucleo/gl/window/glWindow.H>
#include <nucleo/image/sink/ImageSink.H>

#include <stdexcept>

using namespace nucleo ;

int
main(int argc, char **argv) {
  try {
    char *SOURCE = "vnc://127.0.0.1:1?password=mmmmmmmm" ;
    char *RELAY = "" ; // "file:vnc-session.vss" ;

    if (parseCommandLine(argc, argv, "i:r:", "ss", &SOURCE, &RELAY)<0) {
	 std::cerr << std::endl << argv[0] << " [-i rfburl] [-r output]" << std::endl ;
	 exit(1) ;
    }

    long options = glWindow::DOUBLE_BUFFER ;
    long eventmask = glWindow::event::expose
	 | glWindow::event::enter
	 | glWindow::event::leave
	 | glWindow::event::pointerMotion
	 | glWindow::event::buttonPress
	 | glWindow::event::buttonRelease
	 | glWindow::event::keyPress
	 | glWindow::event::keyRelease
	 | glWindow::event::destroy ;
    glWindow *window = glWindow::create(options, eventmask) ;

    window->setTitle("vncrelay") ;
    window->makeCurrent() ;
    window->map() ;

    URI u(SOURCE) ;
    vncImageSource desktop(Image::RGB, u) ;

    ImageSink *relay = 0 ;
    try {
	 relay = ImageSink::create(RELAY) ;
	 relay->start() ;
    } catch (...) {
	 relay = 0 ;
    }

    Image img ;

    unsigned char button_mask = 0 ;
    bool firstOne = true ;

    desktop.start() ;

    for (bool loop=true; loop && desktop.getState()!=ImageSource::STOPPED; ) {
	 bool refresh = false ;

	 ReactiveEngine::step(40) ;

	 glWindow::event e ;
	 while (window->getNextEvent(&e)) {
	   switch( e.type ) {
	   case glWindow::event::destroy:
		loop = false ;
		break ;
	   case glWindow::event::buttonPress:
		button_mask = button_mask | (1<<(e.button-1)) ;
		desktop.pointerEvent(e.x, e.y, button_mask) ;
		break ;
	   case glWindow::event::buttonRelease:
		button_mask = button_mask & (~(1<<(e.button-1))) ;
		desktop.pointerEvent(e.x, e.y, button_mask) ;
		break ;
	   case glWindow::event::pointerMotion:
		desktop.pointerEvent(e.x, e.y, button_mask) ;
    		break ;
	   case glWindow::event::keyPress:
		desktop.keyEvent(e.keysym, true) ;
		break ;
	   case glWindow::event::keyRelease:
		desktop.keyEvent(e.keysym, false) ;
		break ;
	   case glWindow::event::expose:
		refresh = true ;
		break ;
	   case glWindow::event::enter:
	   case glWindow::event::leave:
	   default:
		break ;
	   }
	 }

#if 1
	 if (((ImageSource*)&desktop)->getNextImage(&img)) {
#else
	   // FIXME: This should compile fine, but doesn't...
	   if (desktop.getNextImage(&img)) {
#endif
	   unsigned int width = img.getWidth(), height = img.getHeight() ;
	   if (firstOne) {
		window->setGeometry(width, height) ;
		window->setMinMaxSize(width, height, width, height) ;
		firstOne = false ;
	   }
	   refresh = true ;
	 }

	 if (refresh) {
	   unsigned int width = img.getWidth(), height = img.getHeight() ;

	   glClear(GL_COLOR_BUFFER_BIT) ;
	   glViewport(0,0,width,height) ;
	   glMatrixMode(GL_PROJECTION) ;
	   glLoadIdentity() ;
	   glOrtho(0.0, width-1, 0.0, height-1, 1.0, -1.0) ;
	   glRasterPos2f(0, height-1.01) ;
	   glPixelZoom(1.0, -1.0) ;

	   glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, img.getData()) ;
	   if (relay && relay->getState()!=ImageSink::STOPPED) relay->handle(&img) ;
	   window->swapBuffers() ;
	 }

    }

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (...) {
    std::cerr << "Caught unknown exception" << std::endl ;
  }
}
