/*
 *
 * nucleo/gl/glUtils.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/gl/glUtils.H>
#include <nucleo/image/processing/basic/Transform.H>
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/utils/ByteOrder.H>

#include <cmath>
#include <cstring>

namespace nucleo {

  bool
  glCheckError(const char *context) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
	 std::cerr << "GL Error: " << gluErrorString(err) ;
	 if (context) std::cerr << " (" << context << ")" ;
	 std::cerr << std::endl;
	 return false ;
    }
    return true ;
  }

  void
  glPrintVersionAndExtensions(std::ostream& out) {
    const GLubyte * strVersion = glGetString (GL_VERSION) ;
    const GLubyte * strExtension = glGetString (GL_EXTENSIONS) ;
    out << "version: " << strVersion << std::endl ;
    out << "extensions: " << strExtension << std::endl ;
  }

  bool
  glExtensionIsSupported(const char *extension) {
    int extlen = strlen(extension) ;

    char *p = (char *)glGetString(GL_EXTENSIONS) ;
    if (!p) return false ;

    char *end = p+strlen(p) ;
    while (p<end) {
	 int n = strcspn(p," ") ;
	 if ((extlen==n) && (!strncmp(extension,p,n))) return true ;
	 p +=  n+1 ;
    }

    return false ;
  }

  // -----------------------------------------------------------------------

  void
  glImagePosition(float x, float y) {
    glRasterPos2f(0, 0) ;
    glBitmap(0,0, 0,0, x,y, 0) ;
  }

  // -----------------------------------------------------------------------

  bool
  glImageEncodingParameters(Image::Encoding e,
					   GLenum *format, GLint *internalformat, GLint *alignment, GLenum *type) {

    switch (e) {
    case Image::L:
	 *format = *internalformat = GL_LUMINANCE ;
	 *alignment = 1 ;
	 *type = GL_UNSIGNED_BYTE ;
	 break ;
    case Image::A:
	 *format = *internalformat = GL_ALPHA ;
	 *alignment = 1 ;
	 *type = GL_UNSIGNED_BYTE ;
	 break ;
    case Image::RGB:
	 *format = *internalformat = GL_RGB ;
	 *alignment = 1 ;
	 *type = GL_UNSIGNED_BYTE ;
	 break ;
    case Image::RGB565:
	 *format = *internalformat = GL_RGB ;
	 *alignment = 1 ;
	 // no endian problem; Image::RGB565 has UNSIGNED_SHORT type
	 *type = GL_UNSIGNED_SHORT_5_6_5;
	 break ;
    case Image::RGBA:
	 *format = *internalformat = GL_RGBA ;
	 *alignment = 1 ;
	 *type = GL_UNSIGNED_BYTE ;
	 break ;
    case Image::ARGB:
	 *format = GL_BGRA ;
	 *internalformat = GL_RGBA ;
	 *type = ByteOrder::isLittleEndian() ? GL_UNSIGNED_INT_8_8_8_8 : GL_UNSIGNED_INT_8_8_8_8_REV ;
	 *alignment = 4 ;
	 break ;
#if defined(__APPLE__)
    case Image::YpCbCr422:
	 *format = GL_YCBCR_422_APPLE ;
	 *internalformat = GL_RGBA ;
	 *type = ByteOrder::isLittleEndian() ? GL_UNSIGNED_SHORT_8_8_APPLE : GL_UNSIGNED_SHORT_8_8_REV_APPLE ;
	 *alignment = 2 ;
	 break ;
#endif
    default:
	 // std::cerr << "glImageEncodingParameters: bad image encoding (" << Image::getEncodingName(e) << ")" << std::endl ;
	 return false ;
    }

    return true ;
  }

  // -----------------------------------------------------------------------

  bool
  glScreenCapture(Image *img, Image::Encoding e, bool mirror) {
    GLint viewport[4] ;
    glGetIntegerv(GL_VIEWPORT, viewport) ;
    int x=viewport[0], y=viewport[1] ;
    int width=viewport[2], height=viewport[3] ;

    GLenum format, type ;
    GLint i, a ;
    glImageEncodingParameters(Image::ARGB, &format, &i, &a, &type) ;
    img->prepareFor(width, height, Image::ARGB) ;
    glReadPixels(x, y, width, height, format, type, img->getData()) ;
    img->setTimeStamp() ;

    if (mirror) mirrorImage(img,'v') ;
    convertImage(img, e) ;
    return true ;
  }

  // -----------------------------------------------------------------------

  void
  glSetupTextureImage(Image *img, GLuint texture_target,
				  bool useTexSub,
				  GLint xoffset, GLint yoffset, GLsizei width, GLsizei height) {
    GLenum format=GL_RGB ;
    GLint internalformat=GL_RGB, alignment=1 ;
    GLenum type = GL_UNSIGNED_BYTE ;
    glImageEncodingParameters(img->getEncoding(), &format, &internalformat, &alignment, &type) ;

    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment) ;

    if (useTexSub) {
	 if (!xoffset && !yoffset && !width && !height) {
	   xoffset = yoffset = 0 ;
	   width = img->getWidth() ;
	   height = img->getHeight() ;
	 }
    	 glTexSubImage2D(texture_target,
				  0, // level
				  xoffset, yoffset, width, height,
				  format, type,
				  img->getData()) ;
    } else {
      if (width == 0) width = img->getWidth();
      if (height == 0) height = img->getHeight();
	 glTexImage2D(texture_target,
			    0, // level
			    internalformat,
			    width, height,
			    0, // border
			    format, type,
			    img->getData()) ;
    }

  }


  void
  glSetupDummyTextureImage(Image::Encoding enc,
					  GLuint texture_target,
					  GLsizei width, GLsizei height) {
    GLenum format=GL_RGB ;
    GLint internalformat=GL_RGB, alignment=1 ;
    GLenum type=GL_UNSIGNED_BYTE ;
    glImageEncodingParameters(enc, &format, &internalformat, &alignment, &type) ;

    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment) ;
    glTexImage2D(texture_target,
			  0, // level
			  internalformat,
			  width, height,
			  0, // border
			  format, type,
			  NULL) ;
  }
  
}
