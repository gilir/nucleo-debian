/*
 *
 * tests/test-sgNode.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/gl/window/glWindow.H>
#include <nucleo/gl/scenegraph/sgNode.H>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

using namespace nucleo ;

// --------------------------------------------------------------------------------

class Rectangle : public sgNode {
protected:
  GLfloat x1, y1, x2, y2 ;
  GLfloat red, green, blue ;

  void display(dlPolicy policy) {
    glColor3f(red,green,blue) ;
    glRectf(x1,y1,x2,y2) ;
  }

public:
  Rectangle(std::string name,
		  GLfloat a, GLfloat b, GLfloat c, GLfloat d,
		  GLfloat e, GLfloat f, GLfloat g) : sgNode(name) {
    x1 = a ; y1 = b ; x2 = c ; y2 = d ;
    red = e ; green = f ; blue = g ;
  }

} ;

class Tester : public ReactiveObject {

protected:

  glWindow *window ;
  sgNode *root ;

  void react(Observable *obs) {
    glWindow::event e ;
    while (window->getNextEvent(&e)) {
	 switch (e.type) {
	 case glWindow::event::keyRelease:
	   switch(e.keysym) {
	   case XK_Escape: ReactiveEngine::stop() ;
	   case XK_d: sgNode::debugMode = ! sgNode::debugMode ; break ;
	   case XK_D: root->debug(std::cout) ; break ;
	   default: break ;
	   }
	   break ;
	 case glWindow::event::configure: root->postRedisplay() ; break ;
	 case glWindow::event::expose: root->postRedisplay() ; break ;
	 default: break ;
	 }
    }

    if (root->graphChanged()) {
	 glClear(GL_COLOR_BUFFER_BIT) ;
	 root->displayGraph() ;
	 window->swapBuffers() ;
    }
  }

public:

  Tester(const char *title) {
    long options = glWindow::DOUBLE_BUFFER ;
    long eventmask = glWindow::event::configure
	 | glWindow::event::expose
	 | glWindow::event::keyRelease ;
    window = glWindow::create(options, eventmask) ;
    window->setTitle(title) ;
    window->makeCurrent() ;
    window->map() ;
    subscribeTo(window) ;

    root = new sgNode("root") ;
    Rectangle *rect1 = new Rectangle("rect1", -0.7,-0.7,-0.1,-0.1, 1,0,0) ;
    root->addDependency(rect1) ;
    Rectangle *rect2 = new Rectangle("rect2", -0.3,-0.3,0.2,0.2, 0,1,0) ;
    root->addDependency(rect2) ;
  }

  ~Tester(void) {
    delete window ;
  }

} ;

int
main(int argc, char **argv) {
  try {
    if (parseCommandLine(argc, argv, "", "")<0) {
	 std::cerr << std::endl << argv[0] ;
	 std::cerr << "" << std::endl ;
	 std::cerr << std::endl << std::endl ;
	 exit(1) ;
    }

    Tester tester("test-sgNode") ;
    ReactiveEngine::run() ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
