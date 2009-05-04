/*
 *
 * tests/test-glString.cxx --
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
#include <nucleo/gl/glUtils.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/image/processing/basic/Paint.H>

#include <nucleo/gl/text/glFontManager.H>
#include <nucleo/gl/text/glString.H>

#include <iostream>
#include <stdexcept>
#include <sstream>

using namespace nucleo ;

// --------------------------------------------------------------------------------

class Tester : public ReactiveObject {

protected:

  glWindow *window ;
  glString *glstring ;
  GLfloat angle, scale ;
  bool showBbox ;

  void displayBox(float xmin, float ymin, float xmax, float ymax) {
    glColor3f(0.8,0.8,0.8) ;
    glBegin(GL_LINE_LOOP) ;
    glVertex2f(xmin,ymin) ;
    glVertex2f(xmax,ymin) ;
    glVertex2f(xmax,ymax) ;
    glVertex2f(xmin,ymax) ;
    glEnd() ;
    glColor3f(0.5,0,0) ;
    glBegin(GL_LINES) ;
    glVertex2f(xmin,0) ;
    glVertex2f(xmax,0) ;
    glVertex2f(0,ymin) ;    
    glVertex2f(0,ymax) ;    
    glEnd() ;
    GLfloat o = 3.0 ;
    glBegin(GL_LINE_LOOP) ;
    glVertex2f(-o,-o) ;
    glVertex2f(o,-o) ;
    glVertex2f(o,o) ;
    glVertex2f(-o,o) ;
    glEnd() ;   
  }

  void react(Observable *obs) {
    bool redisplay = false ;

    glWindow::event e ;
    while (window->getNextEvent(&e)) {
	 switch (e.type) {
	 case glWindow::event::keyRelease: {
	   switch (e.keysym) {
	   case XK_Escape: ReactiveEngine::stop() ; break ;
	   case XK_Up: scale*=1.1 ; redisplay=true ; break ;
	   case XK_Down: scale*=0.9 ; redisplay=true ; break ;
	   case XK_Left: angle-- ; redisplay=true ; break ;
	   case XK_Right: angle++ ; redisplay=true ; break ;
	   case XK_Return: angle=0 ; scale=1.0 ; redisplay=true ; break ;
	   case XK_space: showBbox=!showBbox ; redisplay=true ; break ;
	   }
	 } break ;
	 case glWindow::event::configure: {
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

    if (redisplay) {
	 glClear(GL_COLOR_BUFFER_BIT) ;

	 glPushMatrix() ;
	 glScalef(scale,scale,1) ;
	 glRotatef(angle,0,0,1) ;

	 float xmin, ymin, xmax, ymax ;
	 glstring->bbox(&xmin, &ymin, &xmax, &ymax) ;
	 // std::cerr << "bbox: " << xmin << "," << ymin << " - " << xmax << "," << ymax << std::endl ;
	 glTranslatef((int)(-(xmax-xmin)/2), 0, 0) ;
 
	 glTranslatef(0,30,0) ;
	 glColor3f(0,0,0) ;
	 glstring->renderAsTexture() ;
	 if (showBbox) displayBox(xmin, ymin, xmax, ymax) ;

	 glColor3f(0,0,0) ;
	 glTranslatef(0,-60,0) ;
	 glstring->renderAsPixels() ;
	 if (showBbox) displayBox(xmin, ymin, xmax, ymax) ;

	 glPopMatrix() ;

	 window->swapBuffers() ;
    }
  }

public:

  Tester(glWindow *w, glString *s) {
    window = w ;
    subscribeTo(window) ;
    scale = 1.0 ;
    showBbox = false ;
    glstring = s ;
  }

  ~Tester(void) {
  }

} ;

void
sizer(std::string fonturl) {
  glFont *font = glFontManager::getFont(fonturl) ;

  int a, d ;
  font->getLineHeight(&a, &d) ;
  std::cerr << fonturl << ": " << a << " " << d << std::endl ;
}

int
main(int argc, char **argv) {
  try {
    char *FONT="vera:sans-serif" ;
    char *GEOMETRY="400x320" ;

    int firstArg = parseCommandLine(argc, argv, "f:g:", "sss",
							 &FONT, &GEOMETRY) ;

    if (firstArg<0) {
	 std::cerr << std::endl << argv[0] ;
	 std::cerr << " [-f font] [-g geometry]" << std::endl ;
	 std::cerr << std::endl << std::endl ;
	 exit(1) ;
    }

    long options = glWindow::DOUBLE_BUFFER ;
    long eventmask = glWindow::event::configure
	 | glWindow::event::expose
	 | glWindow::event::keyRelease ;
    glWindow *window = glWindow::create(options, eventmask) ;
    window->setTitle("test-glString") ;
    window->setGeometry(GEOMETRY) ;
    window->makeCurrent() ;

#if 0
    sizer(FONT) ;
    sizer("vera:sans-serif") ;
    sizer("vera:serif") ;
    sizer("vera:monospace") ;
#endif

    glClearColor(1,1,1,1) ;
    glEnable(GL_CULL_FACE) ;

    std::cerr << "font: " << FONT << std::endl ;

    glString glstring ;
    if (firstArg!=argc) {
	 std::stringstream tmp ;
	 for (int i=firstArg; i<=argc; ++i)
	   tmp << argv[i] << " " ;
	 std::string message = tmp.str() ;
	 std::cerr << "message: " << message << std::endl ;
	 glFont *font = glFontManager::getFont(FONT) ;
	 glstring << font << message ;
    } else {
	 glFont *sans_serif = glFontManager::getFont("vera:sans-serif") ;
	 glFont *serif = glFontManager::getFont("vera:serif") ;
	 glFont *monospace = glFontManager::getFont("vera:monospace") ;
	 glstring << sans_serif << "sans-serif" << serif << " serif " << monospace << "monospace" ;
    }

    Image tmp1 ;
    glstring.getAsImage(&tmp1, 160,0,0, 5) ;
    convertImage(&tmp1, Image::PNG) ;
    tmp1.saveAs("test-glString-1.png") ;
    std::cerr << "Text saved as test-glString-1.png" << std::endl ;

    Image tmp2 ;
    tmp2.prepareFor(400, 300, Image::ARGB) ;
    paintImageRegion(&tmp2, 0,0,200,300, 255,0,0,255) ;
    paintImageRegion(&tmp2, 200,0,400,300, 0,0,255,255) ;
    glstring.renderInImage(&tmp2, 255,255,255, 100,150) ;
    convertImage(&tmp2, Image::PNG) ;
    tmp2.saveAs("test-glString-2.png") ;
    std::cerr << "Text rendered in test-glString-2.png" << std::endl ;

    Tester tester(window, &glstring) ;

    ReactiveEngine::run() ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
