#ifndef n_glUtils_H
#define n_glUtils_H

/*
 *
 * nucleo/gl/glUtils.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/gl/glIncludes.H>
#include <nucleo/image/Image.H>

namespace nucleo {

  // -----------------------------------------------------------------------

  bool glCheckError(const char *context=0) ;
  void glPrintVersionAndExtensions(std::ostream& out) ;
  bool glExtensionIsSupported(const char *extension) ;

  // -----------------------------------------------------------------------

  void glImagePosition(float x, float y) ;

  bool glImageEncodingParameters(Image::Encoding e,
						   GLenum *format, GLint *internalformat,
						   GLint *alignment, GLenum *type) ;

  bool glScreenCapture(Image *img, Image::Encoding e=Image::ARGB, bool mirror=false) ;

  // -----------------------------------------------------------------------
  // FIXME: these two are now deprecated...

  void glSetupTextureImage(Image *img,
					  GLuint texture_target=GL_TEXTURE_2D,
					  bool useTexSub=false,
					  // The followings must be set if useTexSub is true
					  GLint xoffset=0, GLint yoffset=0, GLsizei width=0, GLsizei height=0) ;

  void glSetupDummyTextureImage(Image::Encoding enc,
						  GLuint texture_target,
						  GLsizei width, GLsizei height);

  // -----------------------------------------------------------------------

}

#endif
