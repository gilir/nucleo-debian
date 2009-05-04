#ifndef n_sgViewpoint_H
#define n_sgViewpoint_H

/*
 *
 * nucleo/gl/scenegraph/sgViewpoint.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/gl/glIncludes.H>
#include <nucleo/gl/scenegraph/sgNode.H>

namespace nucleo {

  // --------------------------------------------------------------------------

  class sgViewpoint : public sgNode {

  protected:

    virtual void setViewpoint(void) = 0 ;

    void display(sgNode::dlPolicy policy) ;

  public:

    static unsigned int glPickingBufferSize ;

    sgViewpoint(std::string name) : sgNode(name) {}

    // Stores all hits with their names and Z values in buffer. Check
    // the glSelectBuffer man page to get information about the data
    // written to the buffer
    int pickAll(int x, int y, GLuint *buffer, GLuint size) ;

    // Stores the names of the closest hit into sbuffer (returns the
    // number of names)
    int pickClosest(int x, int y, GLuint *sbuffer, GLuint size) ;

    void applyTransforms(void) ;

    void unproject(int x, int y,
			    GLuint *sbuffer, int size,
			    GLdouble *ox, GLdouble *oy, GLdouble *oz) ;

    bool project(
	    GLdouble x, GLdouble y, GLdouble z,
	    GLuint *selectionBuffer, int selectionBufferSize,
	    GLdouble *ox, GLdouble *oy, GLdouble *oz);


  } ;

  // --------------------------------------------------------------------------

  class sgOrthoViewpoint : public sgViewpoint {

  private:

    GLdouble _x1, _y1, _x2, _y2, _z1, _z2 ;

    void setViewpoint(void) {
	 glOrtho(_x1,_x2,_y1,_y2,_z1,_z2) ;
    }

  public:

    sgOrthoViewpoint(std::string name="<unknown>", GLdouble x1=-0.5, GLdouble x2=0.5,
				 GLdouble y1=-0.5, GLdouble y2=0.5,
				 GLdouble z1=-0.5, GLdouble z2=0.5) : sgViewpoint(name) {
	 _x1 = x1 ; _x2 = x2 ;
	 _y1 = y1 ; _y2 = y2 ;
	 _z1 = z1 ; _z2 = z2 ;
    }

    void resize(GLdouble x1,GLdouble x2,GLdouble y1,GLdouble y2,GLdouble z1,GLdouble z2) {
	 _x1 = x1 ; _x2 = x2 ;
	 _y1 = y1 ; _y2 = y2 ;
	 _z1 = z1 ; _z2 = z2 ;
	 postRedisplay() ;
    }

    GLdouble getMinX(void) { return _x1 ; }
    GLdouble getMaxX(void) { return _x2 ; }

    GLdouble getMinY(void) { return _y1 ; }
    GLdouble getMaxY(void) { return _y2 ; }

    GLdouble getMinZ(void) { return _z1 ; }
    GLdouble getMaxZ(void) { return _z2 ; }
 
  } ;

  // --------------------------------------------------------------------------

  class sgPerspectiveViewpoint : public sgViewpoint {

  protected:

    GLdouble _ex, _ey, _ez ;
    GLdouble _rx, _ry, _rz ;
    GLdouble _ux, _uy, _uz ;
    GLdouble _near, _far ;

    void setViewpoint(void) {
	 gluPerspective(60.0, 1.0, _near, _far);
	 gluLookAt(_ex, _ey, _ez,
			 _rx, _ry, _rz,
			 _ux, _uy, _uz) ;
    }

  public:

    sgPerspectiveViewpoint(std::string name="<unknown>", GLdouble x=0.0, GLdouble y=0.0, GLdouble z=1.0,
					  GLdouble near=0.001, GLdouble far=15.0) : sgViewpoint(name) {
	 _ex = x ; _ey = y ; _ez = z ;
	 _rx = _ry = _rz = 0.0 ;
	 _ux = 0.0 ; _uy = 1.0 ; _uz = 0.0 ;
	 _near = near ; _far = far ;
    }

    void setNearLimit(GLdouble v) {
	 _near = v ;
	 postRedisplay() ;
    }

    void setFarLimit(GLdouble v) {
	 _far = v ;
	 postRedisplay() ;
    }

    void setEyePoint(GLdouble x, GLdouble y, GLdouble z) {
	 _ex = x ; _ey = y ; _ez = z ;
	 postRedisplay() ;
    }

    void setRefPoint(GLdouble x, GLdouble y, GLdouble z) {
	 _rx = x ; _ry = y ; _rz = z ;
	 postRedisplay() ;
    }

    void setUpVector(GLdouble x, GLdouble y, GLdouble z) {
	 _ux = x ; _uy = y ; _uz = z ;
	 postRedisplay() ;
    }

  } ;

  // --------------------------------------------------------------------------

}

#endif
