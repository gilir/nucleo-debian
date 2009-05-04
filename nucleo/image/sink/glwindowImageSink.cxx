/*
 *
 * nucleo/image/sink/glwindowImageSink.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/sink/glwindowImageSink.H>
#if HAVE_GLX
#include <nucleo/gl/window/glWindow_GLX.H>
#endif

#include <nucleo/image/encoding/Conversion.H>

#include <nucleo/gl/glUtils.H>
#if HAVE_FREETYPE2
#include <nucleo/gl/text/glString.H>
#endif

#include <cstdlib>
#include <cstdio>

#include <stdexcept>
#include <sstream>

namespace nucleo {

  glwindowImageSink::glwindowImageSink(const URI &u) {
    _uri = u ;
    _active = false ;
  }

  glwindowImageSink::~glwindowImageSink() {
    stop() ;
  }

  bool
  glwindowImageSink::start(void) {
    std::string query = _uri.query ;
    std::string title, geometry, filter ;
    URI::getQueryArg(query, "title", &title) ;
    URI::getQueryArg(query, "geometry", &geometry) ;
    URI::getQueryArg(query, "filter", &filter) ;
    bool fullscreen = URI::getQueryArg(query, "fullscreen") ;
    bool nocursor = URI::getQueryArg(query, "nocursor") ;
    bool sync2vbl = URI::getQueryArg(query, "vbl") ;
    _displayFrameRate = URI::getQueryArg(query, "fps") ;

    // --------------------------------------------------------------------------------------
    
    long options = glWindow::DOUBLE_BUFFER ;   
    long eventmask = glWindow::event::configure|glWindow::event::expose|glWindow::event::destroy ;

#if HAVE_GLX
    std::string display = _uri.host ;
    int port = _uri.port ;
    std::stringstream tmp ;
    tmp << display ;
    if (port) tmp << ":" << port ;

    _display = (long *)XOpenDisplay(tmp.str().c_str()) ;
    if (!_display) throw std::runtime_error("glwindowImageSink: can't open display") ;

    Window parent = 0 ;
    URI::getQueryArg(query, "parent", &parent) ;
    if (!parent) eventmask = eventmask|glWindow::event::keyRelease ;

    _window = new glWindow_GLX((Display*)_display, parent, options, eventmask) ;
#else
    eventmask = eventmask|glWindow::event::keyRelease ;
    _window = glWindow::create(options, eventmask) ;
#endif

    bool debugEvents = URI::getQueryArg(query, "debug") ;
    if (debugEvents) _window->debugEvents = true ;

    subscribeTo(_window) ;

    _window->setTitle((char *)title.c_str()) ;

    if (fullscreen) {
	 _window->setFullScreen(true) ;
	 _window->map() ;
	 _fitImage = false ;
    } else if (geometry!="") {
	 _window->setGeometry((char *)geometry.c_str()) ;
	 _window->map() ;
	 _fitImage = false ;
    } else
	 _fitImage = true ;

    _window->setCursorVisible(!nocursor) ;

    if (sync2vbl) _window->syncToVbl() ;

    _window->makeCurrent() ;

    if (filter=="linear")
	 _texture.setFilters(GL_LINEAR, GL_LINEAR) ;

    _saveNextFrame = false ;
    _active = true ;

    frameCount = 0 ; sampler.start() ;
    return true ;
  }

  void
  glwindowImageSink::react(Observable*) {
    // std::cerr << "glwindowImageSink::react" << std::endl ;

    _window->makeCurrent() ;

    glWindow::event e ;
    while (_active && _window->getNextEvent(&e)) {
	 // e.debug(std::cerr) ; std::cerr << std::endl ;

	 switch( e.type ) {
	 case glWindow::event::configure: {
	   _win_width = e.width ;
	   _win_height = e.height ;
	   glViewport(0,0,_win_width,_win_height) ;
	   glMatrixMode(GL_PROJECTION) ;
	   glLoadIdentity() ;
	   glOrtho(0.0, _win_width, 0.0, _win_height, 1.0, -1.0) ;
	   glMatrixMode(GL_MODELVIEW) ;
	   glLoadIdentity() ;
	   refresh() ;
	 } break ;
	 case glWindow::event::expose:
	   refresh() ;
	   break ;
	 case glWindow::event::destroy:
	   stop() ;
	   notifyObservers() ;
	   break ;
	 case glWindow::event::keyRelease:
	   switch (e.keysym) {
	   case XK_Escape: stop() ; notifyObservers() ; break ;
	   case XK_F1: _displayFrameRate = ! _displayFrameRate ; sampler.start() ; break ;
	   case XK_F2: _saveNextFrame = true ; break ;
	   case XK_F3:
		_fitImage = true ; 
		_gcorrect.setCorrectionValue(1.0) ;
		break ;
	   case XK_F4:
		_gcorrect.setCorrectionValue(_gcorrect.getCorrectionValue()-0.1) ;
		break ;
	   case XK_F5:
		_gcorrect.setCorrectionValue(_gcorrect.getCorrectionValue()+0.1) ;
		break ;
	   case XK_F6:
		_window->setFullScreen(true) ;
		break ;
	   case XK_F7:
		_window->setFullScreen(false) ;
		break ;
	   case XK_D:
		_window->debugEvents = ! _window->debugEvents ;
		break ;
	   }
	   break ;
	 default:
	   break ;
	 }
    }
  }

  bool
  glwindowImageSink::handle(Image *img) {
    // std::cerr << "glwindowImageSink::handle" << std::endl ;

    if (!_active) return false ;

    _window->makeCurrent() ;

    if (_saveNextFrame) {
	 Image tmp(*img) ;
	 convertImage(&tmp, Image::JPEG) ;
	 tmp.saveAs("snapshot.jpg") ; 
	 std::cerr << "glwindowImageSink: image saved as snapshot.jpg" << std::endl ;
	 _saveNextFrame = false ;
    }

    if (!_gcorrect.filter(img)) return false ;

    if (!_texture.update(img)) return false ;

    frameCount++ ; sampler.tick() ;

    if (_fitImage) {
	 unsigned int width = img->getWidth() ;
	 unsigned int height = img->getHeight() ;
	 // std::cerr << "Resizing to " << width << "x" << height << std::endl ;
	 _window->setGeometry(width, height) ;
	 _window->setAspectRatio(width, height) ;
	 _window->map() ;
	 _fitImage = false ;
    }

    refresh() ;

    return true ;
  }

  bool
  glwindowImageSink::stop(void) {
    if (_active) {
	 sampler.stop() ;
	 _active = false ;
	 unsubscribeFrom(_window) ;
	 delete _window ;
#if HAVE_GLX
	 XCloseDisplay((Display*)_display) ;
#endif
	 return true ;
    }
    return false ;
  }

  void
  glwindowImageSink::refresh(void) {
    if (!_active || !frameCount) return ;

    glClear(GL_COLOR_BUFFER_BIT) ;

    _texture.display(0,0,_win_width,_win_height) ;

#if HAVE_FREETYPE2
    if (_displayFrameRate) {
	 glString gls ;
	 gls << (int)sampler.average() << " fps" ;
	 glMatrixMode(GL_MODELVIEW) ;
	 glPushMatrix() ;
	 glColor3f(1,1,1) ;
	 glTranslated(5, 5, 0.0) ;
	 gls.renderAsTexture() ;
	 glPopMatrix() ;
    }
#endif

    _window->swapBuffers() ;
  }

}
