#ifndef n_glTextureTile_H
#define n_glTextureTile_H

/*
 *
 * nucleo/gl/texture/glTextureTile.H --
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

  // ------------------------------------------------------------------

  class glTexture ;

  class glTextureTile {

  private:

    static GLint max_pot_size ;
    static bool have_npot_extension ;
    static GLenum npot_target, npot_proxy_target ;
    static GLint max_npot_size ;
    static void initConstants(void) ;

    glTexture *master ;               // original image
 
    unsigned int x, y ;               // position of the upper-left
                                      // corner of the tile in the
                                      // original image (0,0 is the
                                      // upper-left corner of the
                                      // image)
    GLint width, height ;             // dimensions of the tile

    GLuint texture ;
    GLenum tTarget ;
    GLint tWidth, tHeight ;           // dimensions of the texture (>= width,height)
    GLenum tFormat, tType ;
    GLint tAlignment, tInternalFormat ;

    bool fitsIn(GLenum target, GLenum proxytarget, GLint maxsize,
			 bool strict=true, GLint *rw=0, GLint *rh=0) ;

  public:

    static int debugLevel ;

    // texture_rectangle extension policy
    typedef enum {FIRST_CHOICE=1, SECOND_CHOICE=2, DONT_USE=0} trePolicy ;

    glTextureTile(glTexture *master,
			   unsigned int x, unsigned int y,
			   unsigned int width, unsigned int height) ;
    
    void update(Image *img) ;
    bool subUpdate(unsigned int x, unsigned int y, Image *img) ;

    unsigned int getX(void) { return x ; }
    unsigned int getY(void) { return y ; }
    unsigned int getWidth(void) { return width ; }
    unsigned int getHeight(void) { return height ; }

    GLuint getTexture(void) { return texture ; }
    GLenum getTextureTarget(void) { return tTarget ; }
    unsigned int getTextureWidth(void) { return tWidth ; }
    unsigned int getTextureHeight(void) { return tHeight ; }   

    ~glTextureTile(void) ;

  } ;

  // ------------------------------------------------------------------
  
}

#endif
