/*
 *
 * tests/test-glWindow.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/ReactiveObject.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/gl/window/glWindow.H>
#if HAVE_FREETYPE2
#include <nucleo/gl/text/glString.H>
#endif

#include <iostream>
#include <string>
#include <cstdlib>

using namespace nucleo ;

static int win_width, win_height, x=0, y=0 ;
static std::string ks ;

class Tester : public ReactiveObject {

protected:

  glWindow *window1 ;
  glWindow *window2 ;

  void react(Observable *obs) {
    glWindow *window = dynamic_cast<glWindow *>(obs) ;
    if (!window) return ;

    std::string prefix = (obs==window1 ? "#1" : "#2") ;

    window->makeCurrent();

    bool refresh = false ;

    glWindow::event e ;
    while (window->getNextEvent(&e)) {
	 std::cerr << prefix << " " ;
	 switch( e.type ) {
    case glWindow::event::configure:
	 std::cerr << "configure: " << e.width << "x" << e.height << " " << e.x << "," << e.y << std::endl ;
	 glViewport(0,0,e.width, e.height) ;
	 glLoadIdentity() ;
	 glOrtho(0,e.width,0,e.height,0,1) ;
	 win_width = e.width ; win_height = e.height ;
	 refresh = true ;
	 break ;
    case glWindow::event::expose:
	 std::cerr << "expose" << std::endl ;
	 refresh = true ;
	 break ;
    case glWindow::event::destroy:
	 std::cerr << "destroy" << std::endl ;
	 return ;
	 break ;
    case glWindow::event::enter:
	 std::cerr << "enter" << std::endl ;
	 break ;
    case glWindow::event::leave:
	 std::cerr << "leave" << std::endl ;
	 break ;
    case glWindow::event::keyPress:
	 std::cerr << "keyPress: " << e.keysym << " (" << e.keystr << ")" << std::endl ;
	 ks = e.keystr ;
	 refresh = true ;
	 break ;
    case glWindow::event::keyRelease:
	 std::cerr << "keyRelease: " << e.keysym << " (" << e.keystr << ")" << std::endl ;
	 switch(e.keysym) {
	 case XK_Escape: ReactiveEngine::stop() ;
	 case XK_f: window->setFullScreen(true) ; break ;
	 case XK_F: window->setFullScreen(false) ; break ;
	 case XK_1: window->warpCursor(0,0) ; break ;
	 case XK_2: window->warpCursor(10,10) ; break ;
	 case XK_3: window->warpCursor(20,20) ; break ;
	 case XK_c: window->setCursorVisible(false) ; break ;
	 case XK_C: window->setCursorVisible(true) ; break ;
	 default: break ;
	 }
	 break ;
    case glWindow::event::buttonPress:
	 std::cerr << "buttonPress: " << e.button << " " << e.x << "," << e.y << std::endl ;
	 break ;
    case glWindow::event::buttonRelease:
	 std::cerr << "buttonRelease: " << e.button << " " << e.x << "," << e.y << std::endl ;
	 break ;
    case glWindow::event::pointerMotion:
	 std::cerr << "pointerMotion: " << e.x << "," << e.y << std::endl ;
	 x = e.x ; y = win_height - 1 - e.y ;
	 refresh = true ;
	 break ;
    case glWindow::event::wheelMotion:
	 std::cerr << "wheelMotion: " << e.axis << "," << e.delta << std::endl ;
	 break ;
    default: break ;
    }
  }

  if (refresh) {
    glClear(GL_COLOR_BUFFER_BIT) ;
    glPushMatrix() ;
    glColor3f(1.0,1.0,0) ;
    glRectf(x-3, y-3, x+3, y+3) ;

#if HAVE_FREETYPE2
    glString s ;
    s << x << "," << y << " " << ks ;
    glTranslatef(10,10,0) ;
    s.renderAsTexture() ;
    glPopMatrix() ;
#endif

    window->swapBuffers() ;
  }
}

public:

  Tester(bool twoWindows) {
    long options = glWindow::DOUBLE_BUFFER ;
    long eventmask = glWindow::event::configure
	 | glWindow::event::expose
	 | glWindow::event::destroy
	 | glWindow::event::enter
	 | glWindow::event::leave
	 | glWindow::event::keyPress
	 | glWindow::event::keyRelease
	 | glWindow::event::buttonPress
	 | glWindow::event::buttonRelease
	 | glWindow::event::pointerMotion
	 | glWindow::event::wheelMotion ;

    window1 = glWindow::create(options, eventmask) ;
    subscribeTo(window1) ;
    window1->setGeometry(160,120) ;
    window1->setTitle("#1") ;
    window1->map() ;
    window1->makeCurrent();
    glClearColor(0.1,0.5,0.4,1) ;

    window2=0 ;
    if (twoWindows) {
	 window2 = glWindow::create(options, eventmask) ;
	 subscribeTo(window2) ;
	 window2->setGeometry(160,120) ;
	 window2->setTitle("#2") ;
	 window2->map() ;
	 window2->makeCurrent();
	 glClearColor(0.4,0.5,0.1,1) ;
    }
  }

  ~Tester(void) {
    unsubscribeFrom(window1) ;
    delete window1 ;
    if (window2) {
	 unsubscribeFrom(window2) ;
	 delete window2 ;
    }
  }

} ;

int
main(int argc, char **argv) {
  bool USE_TWO_WINDOWS = false ;
  if (parseCommandLine(argc, argv, "2", "b",
				   &USE_TWO_WINDOWS)<0) {
    std::cerr << std::endl << argv[0] << " [-2 (windows)]" << std::endl ;
    exit(1) ;
  }

  Tester tester(USE_TWO_WINDOWS) ;
  ReactiveEngine::run() ;

  return 0 ;
}
