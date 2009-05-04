#ifndef n_glTexture_H
#define n_glTexture_H

/*
 *
 * nucleo/gl/texture/glTexture.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
*/

#include <nucleo/gl/glIncludes.H>
#include <nucleo/gl/texture/glTextureTile.H>

#include <list>
#include <map>

namespace nucleo {

  // ------------------------------------------------------------------

  class glTexture {

  private:

    std::list<glTextureTile *> tiles ;
    static GLUtesselator *tesselator ;

  protected:

    friend class glTextureTile ;

    Image memoryholder ;                 // image buffer holder

    glTextureTile::trePolicy trePolicy ; // should tiles use NPOT extensions?
    unsigned int tileSize ;              // box size of each tile
    GLint minFilter, magFilter ;         // filters used by the tiles
    bool generateMipmaps ;               // should we try to generate mipmaps?
    bool useClientStorage ;              // should we use client storage extensions?

  public:

    // Default values for all glTextures
    static glTextureTile::trePolicy def_trePolicy ; // FIRST_CHOICE
    static unsigned int def_tileSize ;              // 512
    static GLint def_minFilter, def_magFilter ;     // GL_LINEAR
    static bool def_generateMipmaps ;               // yes
    static bool def_useClientStorage ;              // yes

    glTexture(void) ;
    ~glTexture() ;

    void clear(void) ;
    bool load(Image *img) ;
    bool update(Image *img) ;
    bool subUpdate(unsigned int x, unsigned int y, Image *img) ;

    // ------------------------------------------

    unsigned int getWidth(void) { return memoryholder.getWidth() ; }
    unsigned int getHeight(void) { return memoryholder.getHeight() ; }
    TimeStamp::inttype getTimeStamp(void) { return memoryholder.getTimeStamp() ; }

    void setFilters(GLint min, GLint mag) {
	 minFilter = min ;
	 magFilter = mag ;
    }

    // ------------------------------------------

    // Displays the image in a box delimited by the specified
    // coordinates (assumes that top>bottom)
    void display(float left, float bottom, float right, float top,
			  bool preserveAspect=true) ;

    // ------------------------------------------

    typedef enum {NW, N, NE, W, C, E, SW, S, SE} anchor ;

    // Returns the bounding box of the image with 0,0 at the specified
    // anchor
    void getBox(float *left, float *bottom, float *right, float *top, anchor a) ;

    // Displays the image with 0,0 at the specified anchor
    void display(anchor a) ;

    // ------------------------------------------

    struct Point {
	 GLdouble coords[3] ;
	 Point(GLdouble x, GLdouble y, GLdouble z=0.0) {
	   coords[0] = x ;
	   coords[1] = y ;
	   coords[2] = z ;
	 }
    } ;

    typedef std::list<Point> ClipRegion ;

    void displayClipped(anchor a, ClipRegion *region) ;

  } ;

  // ------------------------------------------------------------------

}

#endif
