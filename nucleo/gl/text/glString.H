#ifndef n_glString_H
#define n_glString_H

/*
 *
 * nucleo/gl/text/glString.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
*/

#include <nucleo/image/Image.H>

#include <nucleo/gl/text/glFont.H>

#include <list>

namespace nucleo {

  // ------------------------------------------------------------------

  class glString {
    
  private:

    struct pGlyph {
	 glFont     *font ;
	 FT_UInt    index ;
	 FT_Vector  position ;
	 FT_Glyph   glyph ;
	 glTexture  *texture ;

	 pGlyph(glFont *f, FT_UInt i, FT_Pos x, FT_Pos y, FT_Glyph g) ;
    } ;

    FT_UInt previous ; // last char added to the string
    int x, y ;         // current position
    glFont *font ;     // current font

    std::list<pGlyph*> pglyphs ;

  public:

    glString(void) ;
    ~glString(void) { clear() ; }

    void clear(void) ;

    // Default font is vera:sans-serif (12 pixels)
    void setFont(glFont *f) ;

    glFont *getFont(void) ;

    void append(const char *text, unsigned size) ;

    // ----------------------------------------------------

    typedef enum {CLEAR} command ;

    glString& operator<< (command c) {
	 switch (c) {
	 case CLEAR: clear() ; break ;
	 }
	 return *this ;
    }

    glString& operator<< (glFont *f) {
	 setFont(f) ;
	 return *this ;
    }

    glString& operator<< (char c) {
	 append(&c, 1) ;
	 return *this ;
    }

    glString& operator<< (const char *text) {
	 append(text, strlen(text)) ;
	 return *this ;
    }

    glString& operator<< (std::string text) {
	 append(text.c_str(), text.size()) ;
	 return *this ;
    }

    glString& operator<< (int) ;
    glString& operator<< (unsigned int) ;
    glString& operator<< (long) ;
    glString& operator<< (float) ;
    glString& operator<< (double) ;

    // ----------------------------------------------------

    unsigned int getNbGlyphs(void) ;

    // Returns the space actually used by this string when it is
    // rendered at 0,0
    void bbox(float *xmin, float *ymin, float *xmax, float *ymax) ;

    // Textured text can be scaled (better use fractional factors, however)
    void simplyRenderAsTexture(void) ;  // just render the glyphs
    void renderAsTexture(void) ;        // activates blending and sets
								// the texture env mode to "replace"
    void renderAsTexture(float x, float y, float z=0.0) ; // adds a push/translate/pop

    // This one will work only if one unit is one pixel
    void renderAsPixels(void) ;

    // Image will be converted to ARGB and text will be cropped as expected
    void
    renderInImage(Image *image,
			   unsigned char red, unsigned char green, unsigned char blue,
			   int x, int y) ;

    // Image will be ARGB-encoded (alpha channel is used for antialiasing)
    void
    getAsImage(Image *image,
			unsigned char red, unsigned char green, unsigned char blue,
			unsigned int border=0) ;
    
  } ;

  // ------------------------------------------------------------------

}

#endif
