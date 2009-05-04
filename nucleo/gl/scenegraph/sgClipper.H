#ifndef n_sgClipper_H
#define n_sgClipper_H

/*
 *
 * nucleo/gl/scenegraph/sgClipper.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

namespace nucleo {

  // ------------------------------------------------------------------------

  class sgClipper : public sgNode {

  protected:

    virtual void drawClip(void) = 0 ;

    virtual void drawClipWrapper(bool selectMode, sgNode::dlPolicy policy=NODL) {
	 // Otherwise, dependent objects might be drawn because of other
	 // clippers
	 glClear(GL_STENCIL_BUFFER_BIT);

	 glEnable(GL_STENCIL_TEST) ;

	 glStencilOp(GL_KEEP,GL_KEEP,GL_INCR) ;
	 glStencilFunc(GL_ALWAYS, 0, 0xffffffff) ;
	 glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE) ;
	 glDepthMask(GL_FALSE) ;

	 drawClip() ;

	 glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE) ;
	 glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP) ;
	 glStencilFunc(GL_NOTEQUAL, 0, 0xffffffff) ;
	 glDepthMask(GL_TRUE) ;

	 if (selectMode) sgNode::select() ;
	 else sgNode::display(policy) ;

	 glDisable(GL_STENCIL_TEST) ;
    }

    void display(sgNode::dlPolicy policy) {
	 drawClipWrapper(false, policy) ;
    }

    void select(void) {
	 drawClipWrapper(true) ;
    }

  public:

    sgClipper(std::string name="sgClipper") : sgNode(name) {
    }

    virtual ~sgClipper(void) {
    }

  } ;

  class sgRectClipper : public sgClipper {

  private:

    GLdouble _x1, _y1, _x2, _y2 ;

  protected:

    void drawClip(void) {
	 glColor3f(1,1,1) ;
	 glRectd(_x1,_y1,_x2,_y2) ;
    }

  public:

    sgRectClipper(std::string name, GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2) : sgClipper(name) {
	 _x1 = x1 ; _y1 = y1 ;
	 _x2 = x2 ; _y2 = y2 ;
    }

  } ;

  // ------------------------------------------------------------------------

}

#endif
