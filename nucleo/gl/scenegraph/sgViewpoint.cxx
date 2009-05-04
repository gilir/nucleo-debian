/*
 *
 * nucleo/gl/scenegraph/sgViewpoint.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
*/

#include <nucleo/config.H>

#include <nucleo/gl/scenegraph/sgViewpoint.H>

namespace nucleo {

  unsigned int sgViewpoint::glPickingBufferSize = 1024 ;

  // ------------------------------------------------------------------------

  void
  sgViewpoint::display(sgNode::dlPolicy policy) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();   

    setViewpoint() ;

    glMatrixMode(GL_MODELVIEW) ;
    glLoadMatrixf(_transformations) ;

    for( std::list<sgNode *>::iterator o=_dependencies.begin();
	    o != _dependencies.end(); 
	    ++o ) (*o)->displayGraph(policy) ;
  }

  void
  sgViewpoint::applyTransforms(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();   

    setViewpoint() ;

    glMatrixMode(GL_MODELVIEW) ;
    glLoadMatrixf(_transformations) ;
  }

  // ------------------------------------------------------------------------

  int
  sgViewpoint::pickAll(int x, int y, GLuint *buffer, GLuint size) {
	 glSelectBuffer(size, buffer) ;

	 GLint viewport[4] ;
	 glGetIntegerv(GL_VIEWPORT, viewport) ;

	 glMatrixMode(GL_PROJECTION);
	 glLoadIdentity();
	 gluPickMatrix( (GLdouble) x, (GLdouble) (viewport[3] - y -1), 
				 2.0, 2.0, viewport ) ;
	 setViewpoint() ;

	 glMatrixMode(GL_MODELVIEW) ;
	 glLoadIdentity() ;
	 glMultMatrixf(_transformations) ;

	 /*
	   Removed 28/11/2002 (IHM 2002): VideoWorkspace wasn't able to
	   unproject a point after pick had been called.
	   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ;
	 */

	 glRenderMode(GL_SELECT) ;
	 //std::cerr << "--- sgViewpoint::pickAll: glInitNames ----------------------------" << std::endl ;
	 glInitNames() ;
	 sgNode::select() ;

	 return (int)glRenderMode(GL_RENDER) ;
  }

  // ------------------------------------------------------------------------

  int
  sgViewpoint::pickClosest(int x, int y, GLuint *sbuffer, GLuint size) {
    	 GLuint *tmpbuffer = new GLuint [glPickingBufferSize] ;

	 int hits = pickAll(x,y,tmpbuffer,glPickingBufferSize) ;
	 if (hits<=0) {
	   delete [] tmpbuffer ;
	   return 0 ;
	 }

	 GLuint *selection=0, selectionSize=0 ;
	 GLdouble minimum=0 ;
	 for (GLuint *hit=tmpbuffer, i=0; (GLint)i<hits; i++) {
	   GLuint numNames = *hit ;
	   GLdouble minWinZ = *(hit+1) / 4294967295.0 ;
	   // GLdouble maxWinZ = *(hit+2) / 4294967295.0 ;
	   GLuint *names = hit+3 ;
	   if (i==0 || minWinZ<=minimum) {
		selection = names ;
		selectionSize = numNames ;
		minimum = minWinZ ;
	   }
	   hit += 3+numNames ;
	 }

	 // std::cerr << "--- sgViewpoint::pickClosest: " << std::flush ;
	 for (GLuint n=0; n<size && n<selectionSize; n++) {
	   // std::cerr << selection[n] << " " << std::flush ;
	   sbuffer[n] = selection[n] ;
	 }
	 // std::cerr << std::endl ;
	 delete [] tmpbuffer ;

	 return selectionSize ;    
    }

  void
  sgViewpoint::unproject(int x, int y,
					GLuint *selectionBuffer, int selectionBufferSize,
					GLdouble *ox, GLdouble *oy, GLdouble *oz) {
    GLfloat z = -10.0 ;

    GLint viewport[4] ;
    glGetIntegerv(GL_VIEWPORT, viewport) ;

    applyTransforms() ;

    GLdouble projmatrix[16] ;
    glGetDoublev(GL_PROJECTION_MATRIX, projmatrix) ;

    for (int i=0; i<selectionBufferSize; ++i) {
      sgNode *o = sgNode::lookupId(selectionBuffer[i]) ;
#if DEBUG_LEVEL>=1
	 std::cerr << o->getName() << " < " << std::flush ;
#endif
	 o->applyTransformations() ;
    }
#if DEBUG_LEVEL>=1
    std::cerr << std::endl ;
#endif

    GLdouble mvmatrix[16] ;
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix) ;

    glReadBuffer(GL_FRONT) ;
    glReadPixels(x,viewport[3]-y-1, 1,1, GL_DEPTH_COMPONENT,GL_FLOAT, (GLvoid *)&z) ;

    glReadBuffer(GL_BACK) ;
    gluUnProject(x,viewport[3]-y,z, mvmatrix, projmatrix, viewport, ox, oy, oz) ;

#if DEBUG_LEVEL>=1
    std::cerr << " x=" << x << " y=" << y << " (" << viewport[3]-y << ") z=" << z << std::endl ;
    std::cerr << " ox=" << *ox << " oy=" << *oy << " oz=" << *oz << std::endl ;
#endif

  }

  // ------------------------------------------------------------------------

  bool sgViewpoint::project(
	  GLdouble x, GLdouble y, GLdouble z,
	  GLuint *selectionBuffer, int selectionBufferSize,
	  GLdouble *winX, GLdouble *winY, GLdouble *winZ)
  {
	GLint viewport[4] ;
	glGetIntegerv(GL_VIEWPORT, viewport) ;

	applyTransforms();
	
	GLdouble projmatrix[16] ;
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix) ;

	for (int i=0; i<selectionBufferSize; ++i)
	{
		sgNode *o =  sgNode::lookupId(selectionBuffer[i]) ;
		o->applyTransformations() ;
	}

	GLdouble mvmatrix[16] ;
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix) ;

	//glReadBuffer(GL_FRONT) ;
	glReadBuffer(GL_BACK) ;

	GLint ret;
	ret = gluProject(x, y, z, mvmatrix, projmatrix, viewport,
			 winX, winY, winZ);

	return ret;

  }
  

}
