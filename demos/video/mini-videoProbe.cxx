/*
 *
 * demos/video/mini-videoProbe.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/nucleo.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/utils/SignalUtils.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/processing/difference/SceneChangeDetector.H>
#include <nucleo/image/sink/ImageSink.H>
#include <nucleo/gl/window/glWindow.H>
#include <nucleo/gl/glUtils.H>

#include <stdexcept>
#include <cstdlib>

using namespace nucleo ;

bool signaled=false ;
void exitProc(int s) {
  std::cerr << "signal : " << s << std::endl ;
  signaled = true ; 
}

int
main(int argc, char **argv) {
  try {
    char *SOURCE = "videoin:/anydev/anynode" ;
    char *SINK = "glwindow:?title=mvp-output" ;
    char *CSINK = 0 ;
    bool NOCONTROLVIEW = false ;

    if (parseCommandLine(argc, argv, "i:o:p:c", "sssb", &SOURCE, &SINK, &CSINK, &NOCONTROLVIEW)<0) {
	 std::cerr << std::endl << argv[0] << " [-i source] [-o sink] [-p ctrl-sink] [-c (no control view)]" ;
	 std::cerr << std::endl ;
	 exit(1) ;
    }

    ImageSource *source = ImageSource::create(SOURCE, Image::RGB) ;
    Image img ;
    SceneChangeDetector scd ;
    ImageSink *sink = ImageSink::create(SINK) ;

    glWindow *window = 0 ;
    ImageSink *csink = 0 ;
    Image capture_image ;
    if (!NOCONTROLVIEW) {
	 long options = glWindow::DOUBLE_BUFFER ;
	 long eventmask = glWindow::event::configure
	   | glWindow::event::expose
	   | glWindow::event::keyRelease ;
	 window = glWindow::create(options, eventmask) ;
	 window->setTitle("mvp-control") ;
	 if (CSINK) {
	   csink = ImageSink::create(CSINK) ;
	   csink->start() ;
	 }
    }

    source->start() ;
    sink->start() ;

    trapAllSignals(exitProc) ;
    while (!signaled && sink->getState()!=ImageSink::STOPPED) {
	 ReactiveEngine::step() ;

	 if (source->getNextImage(&img)) {
#if 0
	   int pstate = scd.getState() ;
#endif
	   scd.handle(&img) ;
	   int state = scd.getState() ;
#if 0
	   if (pstate!=state)
		std::cerr << SceneChangeDetector::statenames[state] << std::endl ;
#endif

	   if (window) {
		window->makeCurrent() ;
		unsigned int width=img.getWidth(), height=img.getHeight() ;
		if (source->getFrameCount()==1) {
		  window->setGeometry(width, height, 500, 120) ;
		  glViewport(0,0,width,height) ;
		  glMatrixMode(GL_PROJECTION) ;
		  glLoadIdentity() ;
		  glOrtho(-0.5,0.5,-0.5,0.5,-1.0,1.0) ;
		  glMatrixMode(GL_MODELVIEW) ;
		  glPixelZoom(1,-1) ;
		  glPixelStorei(GL_UNPACK_ALIGNMENT, 1) ;
		  glClearColor(1,1,1,1) ;
		}

		if (state!=SceneChangeDetector::RESET
		    && state!=SceneChangeDetector::IDLE) {		
		  glRasterPos2f(-0.5, 0.5) ;
		  glDrawPixels(img.getWidth(), img.getHeight(),
					GL_RGB, GL_UNSIGNED_BYTE, img.getData());
		} else {
		  glClear(GL_COLOR_BUFFER_BIT) ;
		}

		if (state==SceneChangeDetector::PRESENCE) {
		  glEnable(GL_BLEND) ;
		  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
		  glColor4f(0.0,0.0,0.0,.25) ;
		  double v = (1.0 - scd.timeRemaining())/2.0 ;
		  glRectf(-v, -v, v, v) ;
		  glDisable(GL_BLEND) ;
		}

		if (csink) {
		  glScreenCapture(&capture_image, Image::RGB, true) ;
		  csink->handle(&capture_image) ;
		}
		window->swapBuffers() ;
	   }

	   if (state==SceneChangeDetector::CONFIRMED) {
		std::cerr << "new picture!" << std::endl ;
		sink->handle(&img) ;
	   }
	 }

    }

    delete source ;
    delete sink ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }
}
