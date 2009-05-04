/*
 *
 * nucleo/gl/texture/glTextureTile.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/gl/texture/glTexture.H>

#include <nucleo/gl/glUtils.H>

#include <stdexcept>
#include <sstream>

namespace nucleo {

  int glTextureTile::debugLevel = 0 ;

  GLint glTextureTile::max_pot_size = -1 ;

  bool glTextureTile::have_npot_extension = false ;
  GLenum glTextureTile::npot_target = GL_NONE, glTextureTile::npot_proxy_target=GL_NONE ;
  GLint glTextureTile::max_npot_size = -1 ;
  // http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_non_power_of_two.txt
  // http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_rectangle.txt
  // http://developer.apple.com/opengl/extensions/ext_texture_rectangle.html
  // http://oss.sgi.com/projects/ogl-sample/registry/NV/texture_rectangle.txt

  // http://oss.sgi.com/projects/ogl-sample/registry/SGIS/generate_mipmap.txt

  // ------------------------------------------------------------------------

  static std::string
  getTargetName(GLenum target) {
    switch(target) {
    case GL_TEXTURE_2D: return "GL_TEXTURE_2D" ;
    case GL_PROXY_TEXTURE_2D: return "GL_PROXY_TEXTURE_2D" ;
#if defined(GL_TEXTURE_RECTANGLE_ARB)
    case GL_TEXTURE_RECTANGLE_ARB: return "GL_TEXTURE_RECTANGLE_ARB" ;
    case GL_PROXY_TEXTURE_RECTANGLE_ARB: return "GL_PROXY_TEXTURE_RECTANGLE_ARB" ;
#elif defined(GL_TEXTURE_RECTANGLE_EXT)
    case GL_TEXTURE_RECTANGLE_EXT: return "GL_TEXTURE_RECTANGLE_EXT" ;
    case GL_PROXY_TEXTURE_RECTANGLE_EXT: return "GL_PROXY_TEXTURE_RECTANGLE_EXT" ;
#elif defined(GL_TEXTURE_RECTANGLE_NV)
    case GL_TEXTURE_RECTANGLE_NV: return "GL_TEXTURE_RECTANGLE_NV" ;
    case GL_PROXY_TEXTURE_RECTANGLE_NV: return "GL_PROXY_TEXTURE_RECTANGLE_NV" ;
#endif
    default: {
	 std::stringstream tmp ;
	 tmp << "[" << target << "]" ;
	 return tmp.str() ;
    }
    }
  }

  // ------------------------------------------------------------------------

  void
  glTextureTile::initConstants(void) {
    if (debugLevel>1)
	 std::cerr << "glTextureTile::initConstants: max_pot_size=" << max_pot_size 
			 << " max_npot_size=" << max_npot_size << std::endl ;
    
    if (max_pot_size>=0) return ;

    while (glGetError()!=GL_NO_ERROR) ; // clear previous errors
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_pot_size) ;
    if (glGetError()!=GL_NO_ERROR) {
	 if (debugLevel>1) 
	   std::cerr << "glTextureTile::initConstants: glGetIntegerv failed (GL_MAX_TEXTURE_SIZE)" << std::endl ;
	 max_pot_size = 0 ;
    }
   
    // if ARB_texture_non_power_of_two is supported, GL_TEXTURE_2D can
    // handle NPOT images
    if (glExtensionIsSupported("GL_ARB_texture_non_power_of_two")) return ;
   
    have_npot_extension = false ;

#if defined(GL_TEXTURE_RECTANGLE_ARB)
    if (glExtensionIsSupported("GL_ARB_texture_rectangle")) {
	 npot_target = GL_TEXTURE_RECTANGLE_ARB ;
	 npot_proxy_target = GL_PROXY_TEXTURE_RECTANGLE_ARB ;
	 while (glGetError()!=GL_NO_ERROR) ; // clear previous errors
	 glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, &max_npot_size) ;
	 if (glGetError()!=GL_NO_ERROR) max_npot_size = 0 ;
	 have_npot_extension = true ;
    }
#elif defined(GL_TEXTURE_RECTANGLE_EXT)
    if (!have_npot_extension && glExtensionIsSupported("GL_EXT_texture_rectangle")) {
	 npot_target = GL_TEXTURE_RECTANGLE_EXT ;
	 npot_proxy_target = GL_PROXY_TEXTURE_RECTANGLE_EXT ;
	 while (glGetError()!=GL_NO_ERROR) ; // clear previous errors
	 glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT, &max_npot_size) ;
	 if (glGetError()!=GL_NO_ERROR) max_npot_size = 0 ;
	 have_npot_extension = true ;
    }
#elif defined(GL_TEXTURE_RECTANGLE_NV)
    if (!have_npot_extension && glExtensionIsSupported("GL_NV_texture_rectangle")) {
	 npot_target = GL_TEXTURE_RECTANGLE_NV ;
	 npot_proxy_target = GL_PROXY_TEXTURE_RECTANGLE_NV ;
	 while (glGetError()!=GL_NO_ERROR) ; // clear previous errors
	 glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_NV, &max_npot_size) ;
	 if (glGetError()!=GL_NO_ERROR) max_npot_size = 0 ;
	 have_npot_extension = true ;
    }
#endif

    if (debugLevel>1)
	 std::cerr << "glTextureTile::initConstants: max_pot_size=" << max_pot_size 
			 << " max_npot_size=" << max_npot_size << std::endl ;
  }

  // ------------------------------------------------------------------------

  bool
  glTextureTile::fitsIn(GLenum target, GLenum proxytarget, GLint maxsize,
				    bool strict, GLint *rw, GLint *rh) {
    if (maxsize>0 && (tWidth>maxsize || tHeight>maxsize)) {
	 if (debugLevel>1)
	   std::cerr << "glTextureTile(" << this << "): "
			   << tWidth << "x" << tHeight 
			   << " is too big for " << getTargetName(target)
			   << " (max is " << maxsize << ")"
			   << std::endl ;
	 return false ;
    }

    if (debugLevel>1)
	 std::cerr << "glTextureTile(" << this << "): "
			 << "trying as " << getTargetName(target)
			 << " (" << tWidth << "x" << tHeight << ")... " << std::flush ;

    glTexImage2D(proxytarget, 0,
			  tInternalFormat, tWidth, tHeight, 0, tFormat, tType,
			  master->memoryholder.getData()) ;

    GLint tmpw=0, tmph=0 ;
    glGetTexLevelParameteriv(proxytarget, 0, GL_TEXTURE_WIDTH, &tmpw) ;
    glGetTexLevelParameteriv(proxytarget, 0, GL_TEXTURE_HEIGHT, &tmph) ;

    bool result = false ;
    if (strict)
	 result = (tmpw == width) && (tmph == height) ;
    else
	 result = (tmpw >= width) && (tmph >= height) ;

    if (rw) *rw = tmpw ;
    if (rh) *rh = tmph ;
    if (result) tTarget = target ;

    if (debugLevel>1) 
	 std::cerr << (result?"success":"failed") 
			 << " (" << tmpw << "x" << tmph << ")"
			 << std::endl ;

    return result ;
  }

  // ------------------------------------------------------------------------

  glTextureTile::glTextureTile(glTexture *m,
						 unsigned int xp, unsigned int yp,
						 unsigned int w, unsigned int h) {
    initConstants() ;

    master = m ;
    x = xp ; y = yp ;
    tWidth = width = w ; tHeight = height = h ;

    if (!glImageEncodingParameters(m->memoryholder.getEncoding(),
							&tFormat, &tInternalFormat, &tAlignment, &tType)) {
	 std::cerr << "glTextureTile: glTexture should have checked the image encoding. How did we get there?" << std::endl ;
	 throw std::runtime_error("glTextureTile: can't create texture (bad image encoding)") ;
    }

    glGenTextures(1, &texture) ;

    trePolicy policy = master->trePolicy ;
    if (glExtensionIsSupported("GL_ARB_texture_non_power_of_two"))
	 policy = DONT_USE ;

    bool foundTarget = false ;

    if (policy==DONT_USE || policy==SECOND_CHOICE)
	 foundTarget = fitsIn(GL_TEXTURE_2D, GL_PROXY_TEXTURE_2D, max_pot_size) ;

    if (!foundTarget && have_npot_extension && (policy!=DONT_USE))
	 foundTarget = fitsIn(npot_target, npot_proxy_target, max_npot_size) ;

    if (!foundTarget && (policy==FIRST_CHOICE))
	 foundTarget = fitsIn(GL_TEXTURE_2D, GL_PROXY_TEXTURE_2D, max_pot_size) ;

    if (!foundTarget)
	 // check if OpenGL has a suggestion to make about the appropriate size
	 foundTarget = fitsIn(GL_TEXTURE_2D, GL_PROXY_TEXTURE_2D, max_pot_size,
					  false, &tWidth, &tHeight) ;

    if (!foundTarget) {
	 tWidth=1 ; while (tWidth<width) tWidth*=2 ;
	 tHeight=1 ; while (tHeight<height) tHeight*=2 ;
	 foundTarget = fitsIn(GL_TEXTURE_2D, GL_PROXY_TEXTURE_2D, max_pot_size,
					  false, &tWidth, &tHeight) ;
    }

    while (glGetError()!=GL_NO_ERROR) ; // we might have generated some errors
    if (!foundTarget) throw std::runtime_error("glTextureTile: can't create texture (no target)") ;

    // ------------------------------

    glBindTexture(tTarget, texture) ;

    unsigned char *pixels = master->memoryholder.getData() ;

#ifdef __APPLE__
    // http://developer.apple.com/samplecode/Sample_Code/Graphics_3D/TextureRange.htm
    bool clientstorage = master->useClientStorage && tWidth==width && tHeight==height ;
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, clientstorage ? 1 : 0) ;
    glTexParameterf(tTarget, GL_TEXTURE_PRIORITY, 0.0f) ;  // AGP texturing please
#if HAVE_AGL
    glTextureRangeAPPLE(tTarget, master->memoryholder.getSize(), pixels) ;
    // Use *CACHED* for VRAM, *SHARED* for AGP and *PRIVATE* for the
    // normal texturing path
    glTexParameteri(tTarget, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_SHARED_APPLE) ;
#endif
#endif

    glTexParameteri(tTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) ;
    glTexParameteri(tTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) ;

    bool mipmapped = false ;
    if (tTarget==GL_TEXTURE_2D) {
	 if (master->generateMipmaps) {
	   mipmapped = true ;
	   glTexParameteri(tTarget, GL_GENERATE_MIPMAP_SGIS, GL_TRUE) ;
	   glTexParameteri(tTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR) ;
	   glTexParameteri(tTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST) ;
	 } else {
	   glTexParameteri(tTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST) ;
	   glTexParameteri(tTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST) ;
	 }
    } else {
	 glTexParameteri(tTarget, GL_TEXTURE_MAG_FILTER, master->magFilter) ;
	 glTexParameteri(tTarget, GL_TEXTURE_MIN_FILTER, master->minFilter) ;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, tAlignment) ;
    glPixelStorei(GL_UNPACK_ROW_LENGTH, master->memoryholder.getWidth()) ;
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, x) ;
    glPixelStorei(GL_UNPACK_SKIP_ROWS, y) ;
    
    while (glGetError()!=GL_NO_ERROR) ; // we might have generated some errors

    if (tWidth==width && tHeight==height) {
	 glTexImage2D(tTarget, 0, tInternalFormat,
			    tWidth, tHeight, 0,
			    tFormat, tType, pixels) ;
	 glCheckError("glTexImage2D") ;   
    } else {
	 glTexImage2D(tTarget, 0, tInternalFormat,
			    tWidth, tHeight, 0,
			    tFormat, tType, 0) ;
	 glCheckError("glTexImage2D") ;   
	 glTexSubImage2D(tTarget, 0,
				  0, 0, width, height,
				  tFormat, tType, pixels) ;   
	 glCheckError("glTexSubImage2D") ;
    }

    // ------------------------------

    if (debugLevel) {
	 std::cerr << "glTextureTile(" << this << "): "
			 << getTargetName(tTarget) << " "
			 << width << "x" << height << "@" << x << "," << y
			 << " (" << tWidth << "x" << tHeight ;
	 if (mipmapped) std::cerr << ", mipmapped" ;
	 std::cerr << ") " << std::endl ;
    }
  }

  // ------------------------------------------------------------------------

  void
  glTextureTile::update(Image *img) {
    glBindTexture(tTarget, texture) ;
    glTexSubImage2D(tTarget, 0,
				0, 0, width, height,
				tFormat, tType, img->getData()) ;
  }

  bool
  glTextureTile::subUpdate(unsigned int ux1, unsigned int uy1, Image *uimg) {
    unsigned int ux2 = ux1 + uimg->getWidth() ;
    unsigned int uy2 = uy1 + uimg->getHeight() ;
    unsigned int tx2 = x + width ;
    unsigned int ty2 = y + height ;

    unsigned int x1 = ux1>x ? ux1 : x ;
    unsigned int y1 = uy1>y ? uy1 : y ;
    unsigned int x2 = ux2<tx2 ? ux2 : tx2 ;
    unsigned int y2 = uy2<ty2 ? uy2 : ty2 ;
    
    if (x1>x2 || y1>y2) return false ;

    if (debugLevel) {
	 std::cerr << "glTextureTile::subUpdate " << this << std::endl ;
	 std::cerr << "   " << ux1 << "," << uy1 << "-" << ux2 << "," << uy2
			 << " | " << x << "," << y << "-" << tx2 << "," << ty2
			 << " | " << x1 << "," << y1 << "-" << x2 << "," << y2
			 << std::endl ;
    }

    glBindTexture(tTarget, texture) ;
    glTexSubImage2D(tTarget, 0,
				x1-x, y1-y, x2-x1, y2-y1,
				tFormat, tType, uimg->getData()) ;

    return false ;
  }

  // ------------------------------------------------------------------------

  glTextureTile::~glTextureTile(void) {
    glDeleteTextures(1, &texture) ;
  }

  // ------------------------------------------------------------------------

}
