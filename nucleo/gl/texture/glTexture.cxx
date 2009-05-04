/*
 *
 * nucleo/gl/texture/glTexture.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/gl/texture/glTexture.H>
#include <nucleo/gl/glUtils.H>
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/image/processing/basic/Resize.H>

#include <cmath>

#include <stdexcept>

namespace nucleo {

  GLUtesselator *glTexture::tesselator = 0 ;

  glTextureTile::trePolicy glTexture::def_trePolicy = glTextureTile::FIRST_CHOICE ;
  unsigned int glTexture::def_tileSize = 512 ;
  GLint glTexture::def_minFilter=GL_LINEAR, glTexture::def_magFilter=GL_LINEAR ;
#ifdef __APPLE__
  bool glTexture::def_generateMipmaps = false ; // FIXME 10.4.8
#else
  bool glTexture::def_generateMipmaps = true ;
#endif
  bool glTexture::def_useClientStorage = true ;

  // ------------------------------------------------------------------------

  glTexture::glTexture(void) {
    trePolicy = def_trePolicy ;
    tileSize = def_tileSize ;
    minFilter = def_minFilter ;
    magFilter = def_magFilter ;
    generateMipmaps = def_generateMipmaps ;
    useClientStorage = def_useClientStorage ;
    clear() ;
  }

  glTexture::~glTexture() {
    clear() ;
  }

  // ------------------------------------------------------------------------

  void
  glTexture::clear(void) {
    while (! tiles.empty()) {
	 glTextureTile *t = tiles.front() ;
	 tiles.pop_front() ;
	 delete t ;
    }
  }

  // ------------------------------------------------------------------------

  bool
  glTexture::load(Image *img) {
    clear() ;

    if (useClientStorage) {
	 // Make sure that tiles can set up direct memory access
	 if (img->dataIsLinked()) {
	   // std::cerr << "glTexture: copyDataFrom" << std::endl ;
	   memoryholder.copyDataFrom(*img) ;
	 } else {
	   // std::cerr << "glTexture: stealDataFrom" << std::endl ;
	   memoryholder.stealDataFrom(*img) ;
	 }
    } else {
	 // std::cerr << "glTexture: linkDataFrom" << std::endl ;
	 memoryholder.linkDataFrom(*img) ;
    }

    GLenum tFormat, tType ;
    GLint tAlignment, tInternalFormat ;
    Image::Encoding encoding = memoryholder.getEncoding() ;
    if (!glImageEncodingParameters(encoding,
							&tFormat, &tInternalFormat, &tAlignment, &tType)) {
	 if (encoding==Image::PNG) convertImage(&memoryholder, Image::ARGB) ;
#if defined(__APPLE__)
	 else convertImage(&memoryholder, Image::ARGB) ;
#else
	 else convertImage(&memoryholder, Image::RGB) ;
#endif
    }

    unsigned int width = memoryholder.getWidth() ;
    unsigned int height = memoryholder.getHeight() ;

    try {

	 glTextureTile *t = new glTextureTile(this,0,0,width,height) ;
	 tiles.push_back(t) ;

    } catch(std::runtime_error e) {

	 for (unsigned int oy=0; oy<height; oy+=tileSize-1) {
	   unsigned int fh = (height-oy>=tileSize) ? tileSize : (height-oy) ;
	   for (unsigned int ox=0; ox<width; ox+=tileSize-1) {
		unsigned int fw = (width-ox>=tileSize) ? tileSize : (width-ox) ;
		glTextureTile *t = new glTextureTile(this,ox,oy,fw,fh) ;
		tiles.push_back(t) ;
	   }
	 } 

    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0) ;
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0) ;
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0) ;

    return true ;
  }

  // ------------------------------------------------------------------------

  bool
  glTexture::update(Image *img) {
    Image localimg(*img) ;

    GLenum tFormat, tType ;
    GLint tAlignment, tInternalFormat ;
    Image::Encoding encoding = localimg.getEncoding() ;
    if (!glImageEncodingParameters(encoding,
							&tFormat, &tInternalFormat, &tAlignment, &tType)) {
	 bool ok = false ;
	 if (encoding==Image::PNG) ok = convertImage(&localimg, Image::ARGB) ;
#if defined(__APPLE__)
	 else ok = convertImage(&localimg, Image::ARGB) ;
#else
	 else ok = convertImage(&localimg, Image::RGB) ;
#endif
	 if (!ok) return false ;
    }

    if (tiles.empty()
	   || img->getWidth()!=memoryholder.getWidth()
	   || img->getHeight()!=memoryholder.getHeight()
	   || img->getEncoding()!=memoryholder.getEncoding()) {
	 return load(&localimg) ;
    }

    for (std::list<glTextureTile*>::iterator i=tiles.begin();
	    i!=tiles.end();
	    ++i) (*i)->update(&localimg) ;

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0) ;
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0) ;
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0) ;

    memoryholder.setTimeStamp(localimg.getTimeStamp()) ;
    return true ;
  }

  // ------------------------------------------------------------------------

  bool
  glTexture::subUpdate(unsigned int x, unsigned int y, Image *img) {
    unsigned int iWidth = img->getWidth() ;
    unsigned int iHeight = img->getHeight() ;
    unsigned int width = memoryholder.getWidth() ;
    unsigned int height = memoryholder.getHeight() ;

    if (tiles.empty()) return load(img) ;

    if (!x && !y && iWidth==width && iHeight==height)
	 return update(img) ;

    Image localimg(*img) ;

    if (localimg.getEncoding()!=memoryholder.getEncoding())
	 if (!convertImage(&localimg, memoryholder.getEncoding()))
	   return false ;
    
    bool result = false ;
    for (std::list<glTextureTile*>::iterator i=tiles.begin();
	    i!=tiles.end();
	    ++i) 
	 if ((*i)->subUpdate(x,y,&localimg)) result = true ;

    if (result) memoryholder.setTimeStamp(localimg.getTimeStamp()) ;
    return result ;
  }

  // ------------------------------------------------------------------------

  void
  glTexture::display(float left, float bottom, float right, float top, bool preserveAspect) {
    if (tiles.empty()) return ;

    unsigned int width = memoryholder.getWidth() ;
    unsigned int height = memoryholder.getHeight() ;

    GLfloat boxWidth=right-left, boxHeight=top-bottom ;
    GLfloat oX=left, oY=bottom ;
    GLfloat xScale=boxWidth/(GLfloat)width, yScale=boxHeight/(GLfloat)height ;

    if (preserveAspect) {
	 GLfloat scale = xScale ;
	 if (height*scale>boxHeight) scale = yScale ;

	 xScale = copysignf(scale, xScale) ;
	 yScale = copysignf(scale, yScale) ;
	 oX += (boxWidth-width*xScale)/2.0 ;
	 oY += (boxHeight-height*yScale)/2.0 ;
    }

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glEnable(GL_TEXTURE_GEN_S);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glEnable(GL_TEXTURE_GEN_T);

    glPushMatrix() ;

#if 0
    std::cerr << "glTexture::display: " 
		    << this << " "
		    << width << "x" << height << " "
		    << "[" << left << "," << bottom << "-" << right << ',' << top << "] "
		    << oX << "," << oY << " "
		    << xScale << " " << yScale
		    << std::endl ;
#endif
    glTranslatef(oX, oY, 0) ;
    glScalef(xScale, yScale, 1) ;

    for (std::list<glTextureTile*>::iterator i=tiles.begin();
	    i!=tiles.end();
	    ++i) {
	 glTextureTile *t = (*i) ;
	 
	 GLenum tTarget = t->getTextureTarget() ;
	 GLfloat tWidth = t->getWidth() ;
	 GLfloat tHeight = t->getHeight() ;
	 GLfloat tTextureWidth = t->getTextureWidth() ;
	 GLfloat tTextureHeight = t->getTextureHeight() ;

	 glBindTexture(tTarget, t->getTexture()) ;
	 glEnable(tTarget) ;

	 GLfloat tw=tWidth, th=tHeight ;
	 if (tTarget==GL_TEXTURE_2D) {
	   tw /= tTextureWidth ;
	   th /= tTextureHeight ;
	 }
	 GLfloat left=t->getX(), top=height-t->getY(), bottom=top-tHeight ;

	 GLfloat xPlane[] = { 1.0, 0, 0, 0};
	 GLfloat yPlane[] = { 0, -1.0, 0, tHeight};
	 if (tTarget==GL_TEXTURE_2D) {
	   xPlane[0] /= tTextureWidth ;
	   yPlane[1] /= tTextureHeight ;
	   yPlane[3] /= tTextureHeight ;
	 }

	 glPushMatrix() ;
	 glTranslatef(left,bottom,0) ;
	 glTexGenfv(GL_S, GL_OBJECT_PLANE, xPlane);
	 glTexGenfv(GL_T, GL_OBJECT_PLANE, yPlane);
	 glRectf(0,0,tWidth,tHeight) ;
	 glPopMatrix() ;

	 glDisable(tTarget) ;
    }

    glPopMatrix() ;

    glDisable(GL_TEXTURE_GEN_S) ;
    glDisable(GL_TEXTURE_GEN_T) ;
  }

  // ------------------------------------------------------------------------

  /*
            N
  NW  ------------- NE
      |           |
      |           |
   W  |     C     | E
      |           |
      |           |
  SW  ------------- SE
            S

  */

  void
  glTexture::getBox(float *left, float *bottom, float *right, float *top, anchor a) {
    unsigned int width = memoryholder.getWidth() ;
    unsigned int height = memoryholder.getHeight() ;

    *left = *bottom = 0 ; *right = width ; *top = height ; // SW
    if (a==S || a==C || a==N) {
	 *right = width/2 ;
	 *left = -(width-*right) ;
    }
    if (a==SE || a==E || a==NE) {
	 *left = -(float)width ;
	 *right = 0 ;
    }
    if (a==W || a==C || a==E) {
	 *top = height/2 ;
	 *bottom = -(height-*top) ;
    }
    if (a==NW || a==N || a==NE) {
	 *bottom = -(float)height ;
	 *top = 0 ;
    }
  }

  void
  glTexture::display(glTexture::anchor a) {
    float left, bottom, right, top ;
    getBox(&left, &bottom, &right, &top, a) ;
    display(left, bottom, right, top, false) ;
  }

  // ------------------------------------------------------------------------

  static GLvoid
  _tessVertexCallback(void *vertex_data, void *user_data) {
    GLdouble *vertex = (GLdouble *)vertex_data ;
    GLdouble *texinfo = (GLdouble *)user_data ;

    GLdouble texture[] = {
	 (vertex[0] - texinfo[0])/texinfo[4],
	 (texinfo[3] - vertex[1] + texinfo[1])/texinfo[5]
    } ;
  
    glTexCoord2dv(texture) ; 
    glVertex2dv(vertex) ;
  }

  static GLvoid
  _tessBeginCallback(GLenum type, void *user_data) {
    glBegin(type);
  }

  static GLvoid
  _tessEndCallback(void *user_data) {
    glEnd();
  }

  static GLvoid
  _tessErrorCallback(GLenum errno, void *user_data) {
    std::cerr << "glTexture: tessellator error ("
		    <<  std::hex << errno << std::dec
		    << ", " << gluErrorString(errno) << ")" << std::endl;
  }

  static GLvoid
  _tessCombineCallback(GLdouble coords[3],
				   GLdouble *vetex_data[4],
				   GLfloat weight[4],
				   void **outData, void *user_data) {
    // FIXME: this should be freed "some time after gluTessEndPolygon is called"...
    GLdouble *vertex = new GLdouble[3] ;
    vertex[0] =  coords[0] ;
    vertex[1] =  coords[1] ;
    vertex[2] =  coords[2] ;
    *outData = vertex ;
  }

#if HAVE_AGL
#include <AvailabilityMacros.h>
#if defined(MAC_OS_X_VERSION_10_5)
#define N_GLU_CALLBACK GLvoid (*) ()
#else
#define N_GLU_CALLBACK GLvoid (*) (...)
#endif
#else
#define N_GLU_CALLBACK GLvoid (*) ()
#endif

  void
  glTexture::displayClipped(anchor a, ClipRegion *region) {
    if (!tesselator) {
	 tesselator = gluNewTess();

	 gluTessCallback(tesselator, GLU_TESS_VERTEX_DATA, (N_GLU_CALLBACK)_tessVertexCallback) ;
	 gluTessCallback(tesselator, GLU_TESS_BEGIN_DATA, (N_GLU_CALLBACK)_tessBeginCallback);
	 gluTessCallback(tesselator, GLU_TESS_END_DATA, (N_GLU_CALLBACK)_tessEndCallback);
	 gluTessCallback(tesselator, GLU_TESS_COMBINE_DATA, (N_GLU_CALLBACK)_tessCombineCallback);
	 gluTessCallback(tesselator, GLU_TESS_ERROR_DATA, (N_GLU_CALLBACK)_tessErrorCallback);

	 gluTessProperty(tesselator, GLU_TESS_BOUNDARY_ONLY, GL_FALSE) ;
	 gluTessProperty(tesselator, GLU_TESS_TOLERANCE, 0) ; // default 0
	 gluTessProperty(tesselator, GLU_TESS_WINDING_RULE,GLU_TESS_WINDING_ABS_GEQ_TWO) ;

	 // all the points are in the xy plan
	 gluTessNormal(tesselator, 0, 0, 1) ;
    }

     for (std::list<glTextureTile*>::iterator i=tiles.begin();
	    i!=tiles.end();
	    ++i) {
	 glTextureTile *t = (*i) ;

	 GLenum tTarget = t->getTextureTarget() ;
	 unsigned int tX = t->getX() ;
	 unsigned int tY = t->getY() ;
	 unsigned int tWidth = t->getWidth() ;
	 unsigned int tHeight = t->getHeight() ;

	 glBindTexture(tTarget, t->getTexture()) ;
	 glEnable(tTarget) ;

	 float bLeft, bBottom, tmp ;
	 getBox(&bLeft, &bBottom, &tmp, &tmp, a) ;

	 GLfloat left = bLeft+tX ;
	 GLfloat top = bBottom+memoryholder.getHeight()-tY ;
	 GLfloat bottom = top-tHeight ;
	 GLfloat right = left+tWidth ;
	   
	 GLdouble vertices[4][3] = {
	   {left, bottom, 0.0},
	   {right, bottom, 0.0},
	   {right, top, 0.0},
	   {left, top, 0.0}
	 } ;

	 GLdouble texinfo[] = { left, bottom, tWidth, tHeight, 1.0, 1.0 } ;
	 if (tTarget==GL_TEXTURE_2D) {
	   texinfo[4] = t->getTextureWidth() ;
	   texinfo[5] = t->getTextureHeight() ;
	 }

	 gluTessBeginPolygon(tesselator, texinfo) ;

	 gluTessBeginContour(tesselator) ;
	 for (std::list<Point>::iterator it=region->begin();
		 it != region->end(); ++it) {
	   gluTessVertex(tesselator, (*it).coords, (*it).coords) ;
	 }
	 gluTessEndContour(tesselator) ;
	 
	 gluTessBeginContour(tesselator) ;
	 for (int i=0; i<4; i++)
	   gluTessVertex(tesselator, vertices[i], vertices[i]) ;
	 gluTessEndContour(tesselator) ;

	 gluTessEndPolygon(tesselator) ;

	 glDisable(tTarget) ;
    }

  }
	
  // ------------------------------------------------------------------------
 
}
