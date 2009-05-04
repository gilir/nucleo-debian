#ifndef n_sgNode_H
#define n_sgNode_H

/*
 *
 * nucleo/gl/scenegraph/sgNode.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/gl/glIncludes.H>

#include <iostream>
#include <list>
#include <string>

namespace nucleo {

  // ------------------------------------------------------------------------

  class sgNode {

  public:

    typedef enum {NODL, USE, CREATE} dlPolicy ;

  private:

    GLuint _id ;

  protected:

    std::string _name ;

    GLfloat _transformations[16] ;
    GLfloat _savedTransformations[16] ;
    std::list<sgNode *> _dependencies ;

    bool _changed, _propagateChanges ;
    bool _hidden;

    int _minNbNoChange, _nbNoChange ;

    bool _couldUseADisplayList ;
    GLint _displaylist ;
    
    virtual void display(dlPolicy policy) ;
    virtual void select(void) ;

  public:

    static bool debugMode ;
    static bool debugPushName ;

    // minNbNoChange = minimum number of redisplay without any change
    // before a display list can be created. Use 0 for no display list
    // at all.
    sgNode(std::string name="unknown",
		 int minNbNoChange=3,
		 bool propagateChanges=false) ;

    virtual ~sgNode(void) ;

    // --------------------------------------------------------

    void debug(std::ostream& out, int curdepth=0) const ;

    // --------------------------------------------------------

    std::string &getName() {
	 return _name ;
    }

    std::list<sgNode *> &getDependencies(void) {
	 return _dependencies ;
    }

    virtual void addDependency(sgNode *o) {
	 _dependencies.push_back(o) ;
	 _changed = true ;
    }

    virtual void removeDependency(sgNode *o) {
	 _dependencies.remove(o) ;
	 _changed = true ;
    }

    void postRedisplay(void) {
	 _changed = true ;
    }

    void hide(void) {
	 _hidden = true ;
	 _changed = true ;
    }

    void unHide(void) {
	 _hidden = false;
	 _changed = true ;
    }

    bool isHidden(void) {
      return _hidden;
    }
    
    GLuint getId() const {
	return _id ;
    }

    static GLuint createId(sgNode * const node);
    static sgNode * const lookupId(GLuint id);
    static void destroyId(GLuint id);
 
   // --------------------------------------------------------

    bool graphChanged(void) ;
    void displayGraph(dlPolicy policy=CREATE) ;
    void selectGraph(void) ;

    // --------------------------------------------------------
    // Modelview transformations

    // Reset the transformation matrix to identity
    void resetTransformations(void) ;
    // save the current transformation matrix 
    void saveTransformations(void) ;
    // restore the saved transformation matrix
    void restoreSavedTransformations(void) ;

    void getTransformation(GLfloat *trans);
    void setTransformation(GLfloat trans[16]);
    void setTransformation(sgNode *n);

    // Translation and rotation can be specified locally (t=m*t) or
    // relatively to the parent node (t=t*m)
    void translate(GLfloat dx, GLfloat dy, GLfloat dz) ;
    void translate_rel(GLfloat dx, GLfloat dy, GLfloat dz) ;
    void rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) ;
    void rotate_rel(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) ;

    // Scaling is a diagonal matrix so there's no notion of self or
    // parent reference
    void scale(GLfloat fx, GLfloat fy, GLfloat fz) ;

    // Multiply the current GL matrix by the transformation matrix of
    // the object
    virtual void applyTransformations(void) ;

  } ;

  // ------------------------------------------------------------------------

}

#endif
