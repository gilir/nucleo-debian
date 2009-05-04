#ifndef n_glFont_H
#define n_glFont_H

/*
 *
 * nucleo/gl/text/glFont.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
*/

#include <ft2build.h>
#include FT_FREETYPE_H 
#include FT_GLYPH_H

#include <nucleo/gl/texture/glTexture.H>

#include <map>

namespace nucleo {

  // ------------------------------------------------------------------

  class glFont {

  protected:

    typedef std::map<FT_UInt,FT_Glyph> GlyphMap ;
    typedef std::map<FT_UInt,glTexture*> TextureMap ;

    FT_Face face ;
    FT_UInt pixelSize ;

    FT_Glyph gCache1[256] ;
    GlyphMap gCache2 ;

    glTexture* tCache1[256] ;
    TextureMap tCache2 ;

    glFont(FT_Face face, FT_UInt pixelSize) ;

    friend class glFontManager ;

  public:

    ~glFont(void) ;

    FT_UInt getCharIndex(FT_ULong charCode) ;
    FT_Vector getKerning(FT_UInt previous, FT_UInt index) ;
    void getLineHeight(int *ascend, int *descend) ;
    FT_Glyph getGlyph(FT_UInt index) ;
    glTexture *getTexture(FT_UInt index) ;

  } ;

  // ------------------------------------------------------------------

}

#endif
