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

#include <nucleo/gl/text/glFont.H>

#include <cmath>
#include <stdexcept>
#include <iostream>

namespace nucleo {

  // ------------------------------------------------------------------------

  glFont::glFont(FT_Face f, FT_UInt ps) {
    face = f ;
    pixelSize = ps ;

    FT_Error error = FT_Set_Pixel_Sizes(face, pixelSize, pixelSize) ;
    if (error) std::cerr << "glFont::glFont: unable to set pixel sizes" << std::endl ;

    error = FT_Select_Charmap(face, ft_encoding_unicode) ;
    if (error) std::cerr << "glFont::glFont: unable to select unicode encoding" << std::endl ;

#if 0
    std::cerr << "charmaps (current is " << face->charmap << "):" << std::endl ;
    for(int i = 0; i < face->num_charmaps; i++ ) {
	 std::cerr << "  #" << i << " (" << face->charmaps[i] << ")" ;
	 std::cerr << " platform=" << face->charmaps[i]->platform_id ;
	 std::cerr << " encoding=" << face->charmaps[i]->encoding_id ;
	 std::cerr << std::endl ;
    }
#endif

    for (unsigned int i=0; i<256; ++i) {
	 gCache1[i] = 0 ;
	 tCache1[i] = 0 ;
    }
  }

  glFont::~glFont(void) {
    for (unsigned int i=0; i<256; ++i) {
	 FT_Done_Glyph(gCache1[i]) ;
	 delete tCache1[i] ;
    }
    for (GlyphMap::iterator i=gCache2.begin();
	    i!=gCache2.end();
	    ++i) {
	 FT_Done_Glyph((*i).second) ;
    }
    for (TextureMap::iterator i=tCache2.begin();
	    i!=tCache2.end();
	    ++i) {
	 delete ((*i).second) ;
    }
    FT_Done_Face(face) ;
  }

  FT_UInt
  glFont::getCharIndex(FT_ULong charCode) {
    FT_UInt index = FT_Get_Char_Index(face, charCode) ;
    return index ;
  }

  FT_Vector
  glFont::getKerning(FT_UInt previous, FT_UInt index) {
    FT_Vector delta ;
    delta.x = delta.y = 0 ;
    if (FT_HAS_KERNING(face) && previous && index) {
	 FT_Get_Kerning(face, previous, index, ft_kerning_default, &delta) ;
	 // if (delta.x || delta.y) std::cerr << "kerning: adding " << delta.x << "," << delta.y << std::endl ;
    }
    return delta ;
  }

  void
  glFont::getLineHeight(int *ascend, int *descend) {
    double top = (double)face->size->metrics.y_ppem*face->ascender/face->units_per_EM ;
    double bottom = (double)face->size->metrics.y_ppem*face->descender/face->units_per_EM ;   
    if (ascend) *ascend = (int)ceil(top) ;
    if (descend) *descend = (int)floor(bottom) ;
   }
  
  FT_Glyph
  glFont::getGlyph(FT_UInt index) {
    FT_Glyph glyph = 0 ;

    if (index<256)
	 glyph = gCache1[index] ;
    else {
	 GlyphMap::iterator i = gCache2.find(index) ;
	 if (i!=gCache2.end()) glyph = (*i).second ;	 
    }

    if (glyph) return glyph ;

    FT_Error error = FT_Load_Glyph(face, index, FT_LOAD_DEFAULT);
    if (error) throw std::runtime_error("glFont::getGlyph: FT_Load_Glyph failed") ;
    
    error = FT_Get_Glyph(face->glyph, &glyph) ;
    if (error) throw std::runtime_error("glFont::getGlyph: FT_Get_Glyph failed") ;

    if (glyph->format != ft_glyph_format_bitmap) {
	 error = FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1) ;
	 if (error) throw std::runtime_error("glFont::getGlyph: FT_Glyph_To_Bitmap failed") ;
    }

    if (index<256) 
	 gCache1[index] = glyph ;
    else
	 gCache2[index] = glyph ;
        
    return glyph ;
  }

  glTexture *
  glFont::getTexture(FT_UInt index) {
    glTexture *texture = 0 ;

    if (index<256)
	 texture = tCache1[index] ;
    else {
	 TextureMap::iterator i = tCache2.find(index) ;
	 if (i!=tCache2.end()) texture = (*i).second ;	 
    }

    if (texture) return texture ;

    // std::cerr << "New texture for index #" << index << std::endl ;

    texture = new glTexture() ;
    texture->setFilters(GL_NEAREST, GL_NEAREST) ;

    FT_BitmapGlyph bmglyph = (FT_BitmapGlyph)getGlyph(index) ;

    Image img ;
    img.setEncoding(Image::A) ;
    img.setDims(bmglyph->bitmap.width, bmglyph->bitmap.rows) ;
    img.setData(bmglyph->bitmap.buffer, bmglyph->bitmap.width*bmglyph->bitmap.rows, Image::NONE) ;
    texture->load(&img) ;

    if (index<256) 
	 tCache1[index] = texture ;
    else
	 tCache2[index] = texture ;

    return texture ;
  }

  // ------------------------------------------------------------------------
 
}
