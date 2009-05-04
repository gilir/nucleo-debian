/*
 *
 * tests/test-glTexture.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/TimeKeeper.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/utils/TimeUtils.H>
#include <nucleo/gl/window/glWindow.H>
#include <nucleo/image/source/ImageSource.H>

#include <nucleo/gl/glUtils.H>
#include <nucleo/gl/texture/glTexture.H>
#if HAVE_FREETYPE2
#include <nucleo/gl/text/glFontManager.H>
#include <nucleo/gl/text/glString.H>
#endif

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstdlib>

using namespace nucleo ;

class Tester : public ReactiveObject {

protected:

  ImageSource *source ;
  glWindow *window ;
  GLfloat scale, x, y, angle ;

  Image image ;

  glTexture texture ;
  bool clipped ;
  glTexture::ClipRegion region ;
  GLint displaylist ;

  Chronometer chrono ;
  TimeKeeper *stats ;
  std::string sstats ;
#if HAVE_FREETYPE2
  glString gls ;
#endif

  void react(Observable *obs) {
    bool redisplay = false ;


    if (obs==stats) {
	 std::stringstream tmp ;
	 tmp << chrono.average() << " frames/sec" ;
	 sstats = tmp.str() ;
#if HAVE_FREETYPE2
	 gls << glString::CLEAR << sstats ;
	 redisplay = true ;
#else
	 std::cerr << sstats << std::endl ;
#endif
    }

    glWindow::event e ;
    while (window->getNextEvent(&e)) {
	 switch (e.type) {
	 case glWindow::event::keyRelease: {
	   switch (e.keysym) {
	   case XK_Escape: ReactiveEngine::stop() ; break ;
	   case XK_r: texture.load(&image) ; break ;
	   case XK_Page_Down: scale*=0.9 ; redisplay=true ; break ;
	   case XK_Page_Up: scale*=1.1 ; redisplay=true ; break ;
	   case XK_Left: x++ ; redisplay=true ; break ;
	   case XK_Right: x-- ; redisplay=true ; break ;
	   case XK_Up: y-- ; redisplay=true ; break ;
	   case XK_Down: y++ ; redisplay=true ; break ;
	   case XK_Home: angle+=5 ; redisplay=true ; break ;
	   case XK_End: angle-=5 ; redisplay=true ; break ;
	   }
	 } break ;
	 case glWindow::event::configure: {
	   // std::cerr << "glWindow::event::configure " << e.width << "x" << e.height << std::endl ;
	   glViewport(0,0,e.width,e.height) ;
	   glMatrixMode(GL_PROJECTION) ;
	   glLoadIdentity();  
	   int hw=e.width/2, hh=e.height/2 ;
	   glOrtho(-hw,e.width-hw,-hh,e.height-hh,-1,1) ;
	   glMatrixMode(GL_MODELVIEW) ;
	   glLoadIdentity();   
	   redisplay = true ;
	 } break ;
	 case glWindow::event::expose:
	   redisplay = true ;
	   break ;
	 default: break ;
	 }
    }

    if (source->getState()!=ImageSource::STOPPED && source->getNextImage(&image)) {
	 texture.update(&image) ;
	 redisplay = true ;
	 chrono.tick() ;
    }

    if (redisplay) {
	 glClear(GL_COLOR_BUFFER_BIT) ;

	 glPushMatrix() ;
	 glScalef(scale,scale,scale) ;
	 glTranslatef(x,y,0) ;
	 glRotatef(angle, 0,0,1) ;

	 float hw=125, hh=125 ;

	 glColor3f(0.4,0.4,1) ;
	 glRectf(-hw,-hh,0,hh) ;
	 glColor3f(1,0.4,0.4) ;
	 glRectf(0,-hh,hw,hh) ;

	 glColor4f(1,1,1,1) ;

	 if (displaylist==0) {
	   std::cerr << "Creating display list" << std::endl ;
	   displaylist = glGenLists(1) ;
	   glNewList(displaylist, GL_COMPILE_AND_EXECUTE) ;
	   if (clipped)
		texture.displayClipped(glTexture::C, &region) ;
	   else
		texture.display(-hw,-hh,hw,hh) ;
	   glEndList() ;
	 } else {
	   glCallList(displaylist) ;
	 }

	 glPopMatrix() ;

#if HAVE_FREETYPE2
	 glPushMatrix() ;
	 glTranslatef(-120, -120, 0) ;
	 glColor3f(0,0,0) ;
	 gls.renderAsTexture() ;
	 glPopMatrix() ;
#endif
	 
	 window->swapBuffers() ;
    }
  }

public:

  Tester(ImageSource *s, glWindow *w, bool c) {
    source = s ;
    subscribeTo(source) ;
    source->start() ;
 
    window = w ;
    subscribeTo(window) ;

    float left=-70, bottom=-70, right=70, top=70 ;
    region.push_back(glTexture::Point(left,top,0)) ;
    region.push_back(glTexture::Point(left,bottom,0)) ;
    region.push_back(glTexture::Point(right,bottom,0)) ;
    region.push_back(glTexture::Point(right,0.5*top,0)) ;
    region.push_back(glTexture::Point(0.5*right,top,0)) ;
    displaylist = 0 ;
    clipped = c ;

    scale = 1.0 ; angle = x = y = 0.0 ;

    stats = TimeKeeper::create(3*TimeKeeper::second, true) ;
    subscribeTo(stats) ;
    chrono.start() ;
  }

  ~Tester(void) {
    unsubscribeFrom(stats) ;
    delete stats ;

    unsubscribeFrom(source) ;
    delete source ;

    glDeleteLists(displaylist,1) ;

    unsubscribeFrom(window) ;
    delete window ;
  }

} ;

int
main(int argc, char **argv) {
  try {

    // char *SOURCE="file:/Users/roussel/casa/images/big-images/satellite/brasilia-2741x2745.jpg" ;
    char *SOURCE="file:/Users/roussel/casa/images/debug/3.png" ;
    // char *SOURCE="file:/Users/roussel/casa/images/debug/abc/abc-300x300.png" ;
    // char *SOURCE="file:/Users/roussel/casa/films/famille/nous/tomi-long.vss" ;
    // char *SOURCE="file:/Users/roussel/casa/images/debug/video/maison-320x240.vss" ;

    char *GEOMETRY="400x320" ;
    int DEBUGLEVEL = 0 ;
    int TILE_SIZE = 512 ;
    int NPOT_POLICY = glTextureTile::FIRST_CHOICE ;
    bool MIPMAPS = false ;
    bool CLIPPED = false ;

    if (parseCommandLine(argc, argv, "i:g:d:s:n:mc", "ssiiibb",
					&SOURCE, &GEOMETRY, &DEBUGLEVEL, &TILE_SIZE, &NPOT_POLICY, &MIPMAPS, &CLIPPED)<0) {
	 std::cerr << std::endl << argv[0] ;
	 std::cerr << " [-i source] [-g geometry] [-d debugLevel] [-s tile-size] [-n NPOT-policy]" << std::endl ;
	 std::cerr << std::endl << std::endl ;
	 exit(1) ;
    }

    glTextureTile::debugLevel = DEBUGLEVEL ;

    ImageSource *source = ImageSource::create(SOURCE, Image::PREFERRED) ;

    long options = glWindow::DOUBLE_BUFFER ;
    long eventmask = glWindow::event::configure
	 | glWindow::event::expose
	 | glWindow::event::keyRelease ;
    glWindow *window = glWindow::create(options, eventmask) ;
    window->setTitle("test-glTexture") ;
    window->setGeometry(GEOMETRY) ;
    window->makeCurrent() ;

    glClearColor(1,1,0.8,1) ;

    glEnable(GL_CULL_FACE) ;

    glEnable(GL_BLEND) ;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE) ;

    glTexture::def_tileSize = TILE_SIZE ;
    glTexture::def_trePolicy = (glTextureTile::trePolicy)NPOT_POLICY ;
    glTexture::def_generateMipmaps = MIPMAPS ;
    Tester tester(source, window, CLIPPED) ;

    // glPrintVersionAndExtensions(std::cerr) ; std::cerr << std::endl ;

    std::cerr << "GL_NV_texture_rectangle: " << glExtensionIsSupported("GL_NV_texture_rectangle") << std::endl ;
    std::cerr << "GL_EXT_texture_rectangle: " << glExtensionIsSupported("GL_EXT_texture_rectangle") << std::endl ;
    std::cerr << "GL_ARB_texture_rectangle: " << glExtensionIsSupported("GL_ARB_texture_rectangle") << std::endl ;
    std::cerr << "GL_ARB_texture_non_power_of_two: " << glExtensionIsSupported("GL_ARB_texture_non_power_of_two") << std::endl ;
    std::cerr << std::endl ;

    ReactiveEngine::run() ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
