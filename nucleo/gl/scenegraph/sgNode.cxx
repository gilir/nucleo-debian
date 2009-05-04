/*
 *
 * nucleo/gl/scenegraph/sgNode.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
*/

#include <nucleo/config.H>

#include <nucleo/gl/scenegraph/sgNode.H>

#include <cmath>
#include <cstring>

#if defined (HAVE_TR1_UNORDERED_MAP)
#include <tr1/unordered_map>
typedef std::tr1::unordered_map<GLuint, void *> sgNodeMap;
#elif defined (HAVE_EXT_HASH_MAP)
#include <ext/hash_map>
typedef __gnu_cxx::hash_map<GLuint, void *> sgNodeMap;
#elif defined (HAVE_HASH_MAP)
#include <hash_map>
typedef std::hash_map<GLuint, void *> sgNodeMap; 
#else
#include <map>
typedef std::map<GLuint, void *> sgNodeMap;
#endif

#if defined(__LP64__) || defined(SIZEOF_POINTER_GT_SIZEOF_UINT)
/* 64-bit Linux platforms may be able to set this to 0 assuming there
   is no memory leak and/or brk() can grow reasonably (2^31 nowadays?).  */
#define USE_SG_NODE_MAP 1
#endif


static GLfloat Identity[16] = {
  1.0, 0.0, 0.0, 0.0,
  0.0, 1.0, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0
} ;

// ------------------------------------------------------------------------
// From Mesa

/*
   * Perform a 4x4 matrix multiplication  (product = a x b).
   * Input:  a, b - matrices to multiply
   * Output:  product - product of a and b
   * WARNING: (product != b) assumed
   * NOTE:    (product == a) allowed    
   */

inline void
matmul( GLfloat *product, const GLfloat *a, const GLfloat *b ) {
  /* This matmul was contributed by Thomas Malik */
  GLint i;
#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]
  /* i-te Zeile */
  for (i = 0; i < 4; i++) {
    GLfloat ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
    P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
    P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
    P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
    P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
  }
#undef A
#undef B
#undef P
}

// Generate a 4x4 transformation matrix from glRotate parameters.
static void
gl_rotation_matrix( GLfloat angle, GLfloat x, GLfloat y, GLfloat z,
				GLfloat m[] ) {
  /* This function contributed by Erich Boleyn (erich@uruk.org) */

  GLfloat s = sin( angle * (M_PI/180.0) );
  GLfloat c = cos( angle * (M_PI/180.0) );

  GLfloat mag = sqrt( x*x + y*y + z*z );

  if (mag == 0.0) {	
    memmove(m, Identity, sizeof(GLfloat)*16);
    return;
  }

  x /= mag;
  y /= mag;
  z /= mag;

  GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;

#define M(row,col)  m[col*4+row]

  /*
	*     Arbitrary axis rotation matrix.
	*
	*  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
	*  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
	*  (which is about the X-axis), and the two composite transforms
	*  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
	*  from the arbitrary axis to the X-axis then back.  They are
	*  all elementary rotations.
	*
	*  Rz' is a rotation about the Z-axis, to bring the axis vector
	*  into the x-z plane.  Then Ry' is applied, rotating about the
	*  Y-axis to bring the axis vector parallel with the X-axis.  The
	*  rotation about the X-axis is then performed.  Ry and Rz are
	*  simply the respective inverse transforms to bring the arbitrary
	*  axis back to it's original orientation.  The first transforms
	*  Rz' and Ry' are considered inverses, since the data from the
	*  arbitrary axis gives you info on how to get to it, not how
	*  to get away from it, and an inverse must be applied.
	*
	*  The basic calculation used is to recognize that the arbitrary
	*  axis vector (x, y, z), since it is of unit length, actually
	*  represents the sines and cosines of the angles to rotate the
	*  X-axis to the same orientation, with theta being the angle about
	*  Z and phi the angle about Y (in the order described above)
	*  as follows:
	*
	*  cos ( theta ) = x / sqrt ( 1 - z^2 )
	*  sin ( theta ) = y / sqrt ( 1 - z^2 )
	*
	*  cos ( phi ) = sqrt ( 1 - z^2 )
	*  sin ( phi ) = z
	*
	*  Note that cos ( phi ) can further be inserted to the above
	*  formulas:
	*
	*  cos ( theta ) = x / cos ( phi )
	*  sin ( theta ) = y / sin ( phi )
	*
	*  ...etc.  Because of those relations and the standard trigonometric
	*  relations, it is pssible to reduce the transforms down to what
	*  is used below.  It may be that any primary axis chosen will give the
	*  same results (modulo a sign convention) using thie method.
	*
	*  Particularly nice is to notice that all divisions that might
	*  have caused trouble when parallel to certain planes or
	*  axis go away with care paid to reducing the expressions.
	*  After checking, it does perform correctly under all cases, since
	*  in all the cases of division where the denominator would have
	*  been zero, the numerator would have been zero as well, giving
	*  the expected result.
	*/

  xx = x * x;
  yy = y * y;
  zz = z * z;
  xy = x * y;
  yz = y * z;
  zx = z * x;
  xs = x * s;
  ys = y * s;
  zs = z * s;
  one_c = 1.0F - c;

  M(0,0) = (one_c * xx) + c;
  M(0,1) = (one_c * xy) - zs;
  M(0,2) = (one_c * zx) + ys;
  M(0,3) = 0.0F;

  M(1,0) = (one_c * xy) + zs;
  M(1,1) = (one_c * yy) + c;
  M(1,2) = (one_c * yz) - xs;
  M(1,3) = 0.0F;

  M(2,0) = (one_c * zx) - ys;
  M(2,1) = (one_c * yz) + xs;
  M(2,2) = (one_c * zz) + c;
  M(2,3) = 0.0F;

  M(3,0) = 0.0F;
  M(3,1) = 0.0F;
  M(3,2) = 0.0F;
  M(3,3) = 1.0F;

#undef M
}

// ------------------------------------------------------------------------

namespace nucleo {

  bool sgNode::debugMode = false ;
  bool sgNode::debugPushName = false;

  // ------------------------------------------------------------------------

  void
  sgNode::debug(std::ostream& out, int curdepth) const {
    for (int i=0; i<curdepth; ++i) out << "   " ;
    out << "'" << _name << "' (" << this ;
    if (_displaylist) out << ", dl=" << _displaylist ;
    out << "):" << std::endl ;
    for( std::list<sgNode *>::const_iterator o=_dependencies.begin();
	    o != _dependencies.end(); 
	    ++o ) (*o)->debug(out,curdepth+1) ;
  }
   // ------------------------------------------------------------------------
 
  static sgNodeMap sgNodes;

  GLuint
  sgNode::createId(sgNode * const node) {
#if USE_SG_NODE_MAP
    static GLuint id = 0x66600000; /* FIXME: set to 0 when debugged! */
    sgNodes[++id] = (void *)node;
    if (debugMode)
      std::cout << "sgNode::createId: node " << std::hex << node << ", id " << id << std::endl;
    return id;
#else
    if ((((uintptr_t)node) >> 31) != 0) {
      uintptr_t u = ((uintptr_t)node) >> 31;
      std::cout << "sgNode::createId: got a 64-bit addressed node " << std::hex << node << " " << u << std::endl;
    }
    return (uintptr_t)node;
#endif
  }

  sgNode * const
  sgNode::lookupId(GLuint id) {
#if USE_SG_NODE_MAP
    sgNodeMap::const_iterator it = sgNodes.find(id);
    if (it != sgNodes.end())
      return (sgNode *)(*it).second;
    if (debugMode)
      std::cout << "sgNode::lookupId: id " << std::hex << id << " not found" << std::endl;
    return NULL;
#else
    return (sgNode *)(uintptr_t)id;
#endif
  }

  void
  sgNode::destroyId(GLuint id) {
#if USE_SG_NODE_MAP
    sgNodes.erase(id);
#endif
  }

  // ------------------------------------------------------------------------

  sgNode::sgNode(std::string name, int minNbNoChange, bool propagateChanges) {
    _name.assign(name) ;
    _minNbNoChange = minNbNoChange ;
    _propagateChanges = propagateChanges ;

    _nbNoChange = 0 ;
    _displaylist = 0 ;
    resetTransformations() ;
    memmove(_savedTransformations,Identity,16*sizeof(GLfloat)) ;
    _changed = true ;
    _hidden = false;
    _id = sgNode::createId(this);
  }

  sgNode::~sgNode(void) {
    if (_displaylist) glDeleteLists(_displaylist,1) ;
    sgNode::destroyId(_id);
  }

  // ------------------------------------------------------------------------

  bool
  sgNode::graphChanged(void) {
    if (_hidden) return false;
    bool childChanged = false ;
    for( std::list<sgNode *>::iterator o=_dependencies.begin();
	    o != _dependencies.end(); 
	    ++o ) {
	 sgNode *node = (*o) ;
	 if (node->graphChanged()) childChanged = true ;
    }

    if (_propagateChanges && childChanged) _changed = true ;
    _couldUseADisplayList = (!_changed) && (!childChanged) ;
    return (_changed||childChanged) ;
  }

  // ------------------------------------------------------------------------

  void
  sgNode::displayGraph(dlPolicy policy) {
    if (debugMode) std::cout << "'" << _name << "' (" << this << "): [policy=" << policy << " wish=" << _couldUseADisplayList << " nbnc=" << _nbNoChange << "] " << std::flush ;

    if (_hidden) return;

    bool createDisplayList=false ;

    if (_minNbNoChange) {
	 if (_couldUseADisplayList && (policy!=NODL)) {
	   _nbNoChange++ ;
	   if (_displaylist) {
		if (debugMode) std::cout << "[call list] " << std::endl ;
		glCallList(_displaylist) ;
		return ;
	   }
	   if (policy==CREATE && _nbNoChange>_minNbNoChange) {
		if (debugMode) std::cout << "[create list] " << std::flush ;
		if (!_displaylist) _displaylist = glGenLists(1) ;
		createDisplayList = (_displaylist!=0) ;
	   }
	 } else {
	   _nbNoChange = 0 ;
	   if (_displaylist) {
		glDeleteLists(_displaylist,1) ;
		_displaylist = 0 ;
	   }
	 }
    }
    
    if (createDisplayList) {
	 _displaylist = glGenLists(1) ;
	 if (!_displaylist)
	   createDisplayList = false ;
	 else
	   glNewList(_displaylist, GL_COMPILE_AND_EXECUTE) ;
    }

    if (debugMode) std::cout << "[draw]" << std::endl ;
    glPushMatrix() ;
    glMultMatrixf((const GLfloat *)_transformations) ;
    display(createDisplayList ? USE : policy) ;
    glPopMatrix() ;

    if (createDisplayList) glEndList() ;

    _changed = false ;
  }
  
  void
  sgNode::display(dlPolicy policy) {
    if (_hidden) return;
    for( std::list<sgNode *>::iterator o=_dependencies.begin();
	    o != _dependencies.end(); 
	    ++o ) (*o)->displayGraph(policy) ;
  }

  void
  sgNode::selectGraph(void) {
    //    if (debugMode) std::cout << "select '" << _name << "' (" << this << ")" << std::endl ;
    if (_hidden) return;
    if (debugPushName)
      std::cerr << "sgNode::selectGraph: pushing " << std::hex << this << " " << (sgNode *)this << " " << getId() << std::dec << std::endl ;
    glPushName(getId()) ;
    glPushMatrix() ;
    glMultMatrixf((const GLfloat *)_transformations) ;
    select() ;
    glPopMatrix() ;
    glPopName() ;
  }

  void
  sgNode::select(void) {
    for( std::list<sgNode *>::iterator o=_dependencies.begin();
	    o != _dependencies.end(); 
	    ++o ) (*o)->selectGraph() ;
  }
  
  // ------------------------------------------------------------------------

  void
  sgNode::resetTransformations(void) {
    memmove(_transformations,Identity,16*sizeof(GLfloat)) ;

    _changed = true ;
  }

  void
  sgNode::saveTransformations(void)
  {
    memmove(_savedTransformations,_transformations,16*sizeof(GLfloat)) ;
  }

  void
  sgNode::restoreSavedTransformations(void)
  {
    memmove(_transformations,_savedTransformations,16*sizeof(GLfloat)) ;

    _changed = true ;
  }

  void
  sgNode::getTransformation(GLfloat *trans)
  {
    memmove(trans,_transformations,16*sizeof(GLfloat)) ;
  }

  void
  sgNode::setTransformation(GLfloat trans[16])
  {
    memmove(_transformations,trans,16*sizeof(GLfloat)) ;
    _changed = true ;
  }

  void
  sgNode::setTransformation(sgNode *n)
  {
    GLfloat trans[16];

    n->getTransformation(trans);

    memmove(_transformations,trans,16*sizeof(GLfloat)) ;
    _changed = true ;
  }

  void
  sgNode::translate(GLfloat dx, GLfloat dy, GLfloat dz) {
    _transformations[12] = _transformations[0]*dx + _transformations[4]*dy +_transformations[8]*dz +_transformations[12] ;
    _transformations[13] = _transformations[1]*dx + _transformations[5]*dy +_transformations[9]*dz +_transformations[13] ;
    _transformations[14] = _transformations[2]*dx + _transformations[6]*dy +_transformations[10]*dz +_transformations[14] ;
    _transformations[15] = _transformations[3]*dx + _transformations[7]*dy +_transformations[11]*dz +_transformations[15] ;

    _changed = true ;
  }

  void
  sgNode::translate_rel(GLfloat dx, GLfloat dy, GLfloat dz) {
    GLfloat tmp[16] ;
    memmove(tmp,_transformations,16*sizeof(GLfloat)) ;

    _transformations[0] = tmp[0] + dx*tmp[3] ;
    _transformations[1] = tmp[1] + dy*tmp[3] ;
    _transformations[2] = tmp[2] + dz*tmp[3] ;
    _transformations[3] = tmp[3] ;
    _transformations[4] = tmp[4] + dx*tmp[7] ;
    _transformations[5] = tmp[5] + dy*tmp[7] ;
    _transformations[6] = tmp[6] + dz*tmp[7] ;
    _transformations[7] = tmp[7] ;
    _transformations[8] = tmp[8] + dx*tmp[3] ;
    _transformations[9] = tmp[9] + dy*tmp[3] ;
    _transformations[10] = tmp[10] + dz*tmp[3] ;
    _transformations[11] = tmp[11] ;
    _transformations[12] = tmp[12] + dx*tmp[15] ;
    _transformations[13] = tmp[13] + dy*tmp[15] ;
    _transformations[14] = tmp[14] + dz*tmp[15] ;
    _transformations[15] = tmp[15] ;

    _changed = true ;
  }

  void 
  sgNode::rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    GLfloat rot[16] ;
    gl_rotation_matrix(angle, x, y, z, rot) ;

    GLfloat res[16] ;
    matmul(res, rot, _transformations) ;
    memmove(_transformations,res,16*sizeof(GLfloat)) ;

    _changed = true ;
  }

  void 
  sgNode::rotate_rel(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    GLfloat rot[16] ;
    gl_rotation_matrix(angle, x, y, z, rot) ;

    matmul(_transformations, _transformations, rot) ;

    _changed = true ;
  }

  void
  sgNode::scale(GLfloat fx, GLfloat fy, GLfloat fz) {
    GLfloat m[16] = {fx,0,0,0, 0,fy,0,0, 0,0,fz,0, 0,0,0,1} ;

    matmul(_transformations, _transformations, m) ;

    _changed = true ;
  }

  void
  sgNode::applyTransformations(void) {
    //    std::cerr << "applyTransformations " << getName() << std::endl ;
    glMultMatrixf((const GLfloat *)_transformations) ;
  }

}
