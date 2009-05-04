/*
 *
 * demos/misc/paz.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/nucleo.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/TimeKeeper.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/utils/TimeUtils.H>
#include <nucleo/gl/window/glWindow.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/gl/texture/glTexture.H>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstdlib>

using namespace nucleo ;

class PanAndZoom : public ReactiveObject {

protected:

  GLfloat scale, origin_x, origin_y ;

  int last_x, last_y; // last mouse position
  double interest_x, interest_y; // last click position (world)
  unsigned int winWidth, winHeight;
  enum {IDLE, PANNING, ZOOMIN, ZOOMOUT} state ;
  TimeKeeper *animator ;

  void react(Observable *) {
    if (state==ZOOMIN || state==ZOOMOUT) {
	 double factor = (state==ZOOMIN) ? 1.1 : 0.9 ;
	 double dscale = scale*factor - scale ;
	 double f = dscale/(scale+dscale) ;
	 origin_x += (interest_x - origin_x) * f ;
	 origin_y += (interest_y - origin_y) * f ;
	 scale += dscale ;
	 notifyObservers() ;
    }
  }

public:

  PanAndZoom(void) {
    animator = TimeKeeper::create() ;
    subscribeTo(animator) ;
    scale = 1.0 ; origin_x = origin_y = 0.0 ;
    state = IDLE ;
  }

  ~PanAndZoom(void) {
    delete animator ;
  }

  unsigned int getWinWidth(void) { return winWidth ; }
  unsigned int getWinHeight(void) { return winHeight ; }

  void setViewpoint(void) {
    glLoadIdentity() ;
    glScalef(scale,scale,1) ;
    glTranslatef(-origin_x,-origin_y,0) ;
  }

  void handleWindowEvent(glWindow::event *e) {
    switch (e->type) {
    case glWindow::event::configure:
	 glViewport(0,0,e->width,e->height) ;
	 glMatrixMode(GL_PROJECTION) ;
	 glLoadIdentity();  
	 glOrtho(0,e->width, 0,e->height, -1,1) ;
	 glMatrixMode(GL_MODELVIEW) ;
	 glLoadIdentity();   
	 winWidth = e->width ;
	 winHeight = e->height ;
	 notifyObservers() ;
	 break ;
    case glWindow::event::buttonPress:
	 last_x = e->x ; last_y = e->y ;
	 if (e->button==1) {
	   state=PANNING ;
	 } else {
	   interest_x = e->x/scale + origin_x;
	   interest_y = (winHeight-e->y)/scale + origin_y;
	   state = (e->button==2) ? ZOOMIN : ZOOMOUT ;
	   animator->arm(100*TimeKeeper::millisecond, true) ;
	 }
	 break ;
    case glWindow::event::pointerMotion: {
	 // std::cerr << "state=" << state << std::endl ;
	 int dx=e->x-last_x, dy=e->y-last_y ;
	 last_x = e->x ; last_y = e->y ;
	 if (state==PANNING) {
	   origin_x -= dx/scale;
	   origin_y += dy/scale;
	 } else if (state!=IDLE) {
	   interest_x = e->x/scale + origin_x;
	   interest_y = (winHeight-e->y)/scale + origin_y;
	 }
	 notifyObservers() ;
    } break ;
    case glWindow::event::buttonRelease:
	 animator->disarm() ;
	 last_x = e->x ; last_y = e->y ;
	 state = IDLE ;
	 break ;
    default:
	 break ;
    }
  }

} ;

class Tester : public ReactiveObject {

protected:

  ImageSource *source ;
  Image image ;
  glTexture texture ;

  glWindow *window ;
  bool redisplay ;

  PanAndZoom paz ;

  void react(Observable *obs) {
    if (obs==&paz) redisplay = true ;

    if (source->getNextImage(&image)) {
	 if (source->getFrameCount()==1) {
	   std::stringstream tmp ;
	   tmp << image.getWidth() << "x" << image.getHeight() ;
	   std::string stmp = tmp.str() ;
	   window->setTitle(stmp.c_str()) ;
	 }
	 texture.update(&image) ;
	 redisplay = true ;
    }

    glWindow::event e ;
    while (window->getNextEvent(&e)) {
	 if (e.type==glWindow::event::expose)
	   redisplay = true ;
	 else if (e.type==glWindow::event::keyRelease && e.keysym==XK_Escape)
	   ReactiveEngine::stop() ;
	 else
	   paz.handleWindowEvent(&e) ;
    }
       
    if (redisplay) {
	 glClear(GL_COLOR_BUFFER_BIT) ;
	 paz.setViewpoint() ;
	 glColor4f(1,1,1,1) ;
	 texture.display(0,0,paz.getWinWidth(),paz.getWinHeight()) ;
	 window->swapBuffers() ;
	 redisplay = false ;
    }
  }

public:

  Tester(ImageSource *s, glWindow *w) {
    source = s ;
    subscribeTo(source) ;
    source->start() ;

    window = w ;
    subscribeTo(window) ;
    subscribeTo(paz) ;

    redisplay = false ;
  }

  void postRedisplay(void) {
    redisplay = true ;
    selfNotify() ;
  }

} ;

int
main(int argc, char **argv) {
  try {
    char *SOURCE="/Users/roussel/casa/images/big-images/satellite/brasilia-2741x2745.jpg" ;
    char *GEOMETRY="400x320" ;
    int DEBUGLEVEL = 0 ;
    int TILE_SIZE = 256 ;
    int NPOT_POLICY = glTextureTile::FIRST_CHOICE ;

    if (parseCommandLine(argc, argv, "i:g:d:s:n:", "ssiii",
					&SOURCE, &GEOMETRY, &DEBUGLEVEL, &TILE_SIZE, &NPOT_POLICY)<0) {
	 std::cerr << std::endl << argv[0] ;
	 std::cerr << " [-i source] [-g geometry] [-d debugLevel] [-s tile-size] [-n NPOT-policy]" << std::endl ;
	 std::cerr << std::endl << std::endl ;
	 exit(1) ;
    }

    glTextureTile::debugLevel = DEBUGLEVEL ;

    ImageSource *source = ImageSource::create(SOURCE, Image::RGB) ;

    long options = glWindow::DOUBLE_BUFFER ;
    long eventmask = glWindow::event::configure
	 | glWindow::event::pointerMotion
	 | glWindow::event::buttonPress
	 | glWindow::event::buttonRelease
	 | glWindow::event::expose
	 | glWindow::event::keyRelease ;
    glWindow *window = glWindow::create(options, eventmask) ;
    window->setGeometry(GEOMETRY) ;
    window->makeCurrent() ;
    window->emulateThreeButtonMouse(true) ;

    // glClearColor(1,1,1,1) ;
    glClearColor(1,1,0.8,1) ;
    glEnable(GL_CULL_FACE) ;

    glTexture::def_tileSize = TILE_SIZE ;
    glTexture::def_trePolicy = (glTextureTile::trePolicy)NPOT_POLICY ;
    Tester tester(source, window) ;

    ReactiveEngine::run() ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
