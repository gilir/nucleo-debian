/*
 *
 * nucleo/gl/text/glString.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
*/

#include <nucleo/gl/text/glString.H>
#include <nucleo/gl/text/glFontManager.H>
#include <nucleo/gl/texture/glTexture.H>
#include <nucleo/gl/glUtils.H>
#include <nucleo/image/processing/basic/Paint.H>
#include <nucleo/image/encoding/Conversion.H>

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace nucleo {

  // -----------------------------------------------------------------------

  glString::pGlyph::pGlyph(glFont *f, FT_UInt i, FT_Pos x, FT_Pos y, FT_Glyph g) {
    font = f ;
    index = i ;
    position.x = x ; position.y = y ;
    glyph = g ;
    texture = 0 ;
  }

  // ---------------------------------------------------------------------------------

  glString::glString(void) {
    font = 0 ;
    clear() ;
  }

  void
  glString::clear(void) {
    previous = 0 ;
    x = y = 0 ;
    while (! pglyphs.empty()) {
	 pGlyph *pg = pglyphs.front() ;
	 pglyphs.pop_front() ;
	 delete pg ;
    }
  }

  void
  glString::setFont(glFont *f) {
    font = f ;
    previous = 0 ;
  }

  glFont *
  glString::getFont(void) {
    if (!font) setFont(glFontManager::getFont("vera:sans-serif?size=12")) ;
    return font ;
  }

  void
  glString::append(const char *text, unsigned size) {
    if (!font) setFont(glFontManager::getFont("vera:sans-serif?size=12")) ;

    for (unsigned int n=0; n<size; ++n) {
	 // This charCode will never be a full Unicode character...
	 FT_ULong charCode = (unsigned char)text[n] ;

	 // std::cerr << "n=" << n << " text[n]='" << text[n] << "' charCode=" << charCode << std::endl ;
	 FT_UInt index = font->getCharIndex(charCode) ;
	 if (previous && index) {
	   FT_Vector delta = font->getKerning(previous, index) ;
	   x += delta.x >> 6 ;
	   y += delta.y >> 6 ;
	 }

	 try {
	   FT_Glyph glyph = font->getGlyph(index) ;
	   pglyphs.push_back(new pGlyph(font, index, x, y, glyph)) ;

	   x += glyph->advance.x >> 16 ;
	   y += glyph->advance.y >> 16 ;

	   previous = index ;
	 } catch (std::runtime_error e) {
	   std::cerr << e.what() << " ('" << text[n] << "', " << charCode << ")" << std::endl ;
	 }
    }
  }

  // ---------------------------------------------------------------------------------

  unsigned int
  glString::getNbGlyphs(void) {
    return pglyphs.size() ;
  }

  void
  glString::bbox(float *xmin, float *ymin, float *xmax, float *ymax) {
    *xmin = *ymin =  65000 ;
    *xmax = *ymax = -65000 ;

    for (std::list<pGlyph*>::const_iterator i=pglyphs.begin();
	    i!=pglyphs.end();
	    ++i) {
	 pGlyph *pg = (*i) ;

	 FT_BBox b ;
	 b.xMin = b.yMin = 65000 ;
	 b.xMax = b.yMax = -65000 ;
	 FT_Glyph_Get_CBox( pg->glyph, ft_glyph_bbox_pixels, &b );

	 b.xMin += pg->position.x ;
	 b.xMax += pg->position.x ;
	 b.yMin += pg->position.y ;
	 b.yMax += pg->position.y ;

	 if (b.xMin < *xmin) *xmin = b.xMin;
	 if (b.yMin < *ymin) *ymin = b.yMin;
	 if (b.xMax > *xmax) *xmax = b.xMax;
	 if (b.yMax > *ymax) *ymax = b.yMax;
    }

    if ( *xmin > *xmax ) *xmin = *ymin = *xmax = *ymax = 0;
  }
  
  // ---------------------------------------------------------------------------------

  void
  glString::simplyRenderAsTexture(void) {
    if (pglyphs.empty()) return ;

    for (std::list<pGlyph*>::const_iterator i=pglyphs.begin();
	    i!=pglyphs.end();
	    ++i) {
	 pGlyph *pg = (*i) ;

	 FT_BitmapGlyph bmglyph = (FT_BitmapGlyph)pg->glyph ;

	 if (bmglyph->bitmap.width && bmglyph->bitmap.rows) {
	   if (!pg->texture) pg->texture = pg->font->getTexture(pg->index) ;
	   GLfloat x=pg->position.x+bmglyph->left, y=pg->position.y-bmglyph->bitmap.rows+bmglyph->top ;
	   pg->texture->display(x,y,x+bmglyph->bitmap.width,y+bmglyph->bitmap.rows) ;
	 }
    }
  }   

  void
  glString::renderAsTexture(void) {
    if (pglyphs.empty()) return ;

    GLboolean blendEnabled ;
    glGetBooleanv(GL_BLEND, &blendEnabled) ;
    if (!blendEnabled) glEnable(GL_BLEND);

    GLint texEnvMode ;
    glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texEnvMode) ;
    if (texEnvMode!=GL_REPLACE) glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE) ;

    GLint blendSrc, blendDst ;
    glGetIntegerv(GL_BLEND_SRC, &blendSrc) ;
    glGetIntegerv(GL_BLEND_DST, &blendDst) ;
    if (blendSrc!=GL_SRC_ALPHA || blendDst!=GL_ONE_MINUS_SRC_ALPHA)
	 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;

    simplyRenderAsTexture() ;

    if (blendSrc!=GL_SRC_ALPHA || blendDst!=GL_ONE_MINUS_SRC_ALPHA) glBlendFunc(blendSrc, blendDst) ;
    if (texEnvMode!=GL_REPLACE) glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texEnvMode) ;
    if (!blendEnabled) glDisable(GL_BLEND) ;
  }

  void
  glString::renderAsTexture(float x, float y, float z) {
    glPushMatrix() ;
    glTranslatef(x,y,z) ;
    renderAsTexture() ;
    glPopMatrix() ;
  }  

  // ------------------------------------------------------------------------

  void
  glString::renderAsPixels(void) {
    if (pglyphs.empty()) return ;

    GLfloat zx, zy, color[4], r, g, b ;
    GLboolean blendEnabled ;
    glGetFloatv(GL_ZOOM_X, &zx) ;
    glGetFloatv(GL_ZOOM_Y, &zy) ;
    glGetFloatv(GL_CURRENT_COLOR, color) ;
    glGetFloatv(GL_RED_BIAS, &r) ;
    glGetFloatv(GL_GREEN_BIAS, &g) ;
    glGetFloatv(GL_BLUE_BIAS, &b) ;
    glGetBooleanv(GL_BLEND, &blendEnabled) ;

    glPixelTransferf(GL_RED_BIAS,color[0]) ;
    glPixelTransferf(GL_GREEN_BIAS,color[1]) ;
    glPixelTransferf(GL_BLUE_BIAS,color[2]) ;
    if (blendEnabled!=GL_TRUE) glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1) ;
    glPixelZoom(1.0, -1.0) ;

    for (std::list<pGlyph*>::const_iterator i=pglyphs.begin();
	    i!=pglyphs.end();
	    ++i) {
	 pGlyph *pg = (*i) ;

	 FT_BitmapGlyph bmglyph = (FT_BitmapGlyph)pg->glyph ;

	 GLfloat bmx = pg->position.x + bmglyph->left ;
	 GLfloat bmy = bmglyph->top ;
	 GLsizei bmw = bmglyph->bitmap.width ;
	 GLsizei bmh = bmglyph->bitmap.rows ;

	 // This works only if rpx,rpy is a valid raster position
	 // (i.e. in the viewport). In this case, the glyph will be
	 // displayed if its center is in the viewport. How can we get a
	 // better valid raster position?
	 GLsizei ox=bmw/2, oy=bmh/2 ;
	 GLfloat rpx = bmx + ox ;
	 GLfloat rpy = bmy + oy ;
	 glRasterPos2f(rpx, rpy) ;
	 glBitmap(0,0, 0,0, -ox,-oy, NULL) ;

	 glDrawPixels(bmw, bmh,
			    GL_ALPHA, GL_UNSIGNED_BYTE,
			    bmglyph->bitmap.buffer) ;
    }

    if (blendEnabled!=GL_TRUE) glDisable(GL_BLEND) ;
    glPixelTransferf(GL_RED_BIAS,r) ;
    glPixelTransferf(GL_GREEN_BIAS,g) ;
    glPixelTransferf(GL_BLUE_BIAS,b) ;
    glPixelZoom(zx, zy) ;    
  }

  // ------------------------------------------------------------------------

  static void
  drawGlyphInARGBImage(bool blend,
				   unsigned char *src, unsigned sWidth, unsigned sHeight,
				   unsigned char fgR, unsigned char fgG, unsigned char fgB,
				   unsigned char *dst, unsigned dWidth, unsigned dHeight,
				   int dX, int dY) {
    int sX=0, sY=0, width=sWidth, height=sHeight ;

    if (dX<0) { sX-=dX ; width+=dX ; dX=0 ; }
    if (dY<0) { sY-=dY ; height+=dY ; dY=0 ; }

    if ( (unsigned int)dX>=dWidth || (unsigned int)dY>=dHeight ) return ;

    if ((unsigned int)(dX+width)>=dWidth) { width=dWidth-dX ; }
    if ((unsigned int)(dY+height)>=dHeight) { height=dHeight-dY ; }

#if 0
    std::cerr << "drawGlyphInARGBImage: " << sX << "," << sY << " (" << sWidth << "x" << sHeight << ")" ;
    std::cerr << " -- " << width << "x" << height << " --> " ;
    std::cerr << dX << "," << dY << " (" << dWidth << "x" << dHeight << ")" << std::endl ;
#endif
  
    unsigned char glyphColor[] = {fgR, fgG, fgB} ;

    unsigned char *pSrc = src + (sY*sWidth + sX) ;   // ALPHA is 1 bpp
    if (blend) {
	 for (int row=0; row<height; ++row) {
	   unsigned char *pDst = dst + ((dY+row)*dWidth + dX)*4 ; // ARGB is 4 bpp
	   for (int column=0; column<width; ++column) {
		unsigned char sAlpha = *pSrc++ ;
		unsigned char dAlpha = 255-sAlpha ;
		pDst++ ; // leave the alpha component as it is
		for (unsigned int component=0; component<3; ++component) {
		  float c = (glyphColor[component]*sAlpha + pDst[component]*dAlpha) / 255.0 ;
		  pDst[component] = (unsigned char)c ;
		}
		pDst += 3 ;
	   }
	 }
    } else {
	 for (int row=0; row<height; ++row) {
	   unsigned char *pDst = dst + ((dY+row)*dWidth + dX)*4 ; // ARGB is 4 bpp
	   for (int column=0; column<width; ++column) {
		*pDst++ = *pSrc++ ; 
		memmove(pDst, glyphColor, 3) ;
		pDst += 3 ;
	   }
	 }
    }

  }

  void
  glString::renderInImage(Image *image,
					 unsigned char red, unsigned char green, unsigned char blue,
					 int x, int y) {
    if (pglyphs.empty()) return ;

    if (image->getEncoding()!=Image::ARGB) convertImage(image, Image::ARGB) ;

    unsigned char *dst = image->getData() ;
    unsigned dWidth = image->getWidth() ;
    unsigned dHeight = image->getHeight() ;

    for (std::list<pGlyph*>::const_iterator i=pglyphs.begin();
	    i!=pglyphs.end();
	    ++i) {
	 pGlyph *pg = (*i) ;
	 FT_BitmapGlyph bmglyph = (FT_BitmapGlyph)pg->glyph ;

	 if ((!bmglyph->bitmap.width) || (!bmglyph->bitmap.rows)) continue ;

	 unsigned char *src = bmglyph->bitmap.buffer ;
	 unsigned sWidth = bmglyph->bitmap.width ;
	 unsigned sHeight = bmglyph->bitmap.rows ;

	 int dX = x + pg->position.x + bmglyph->left ;
	 int dY = dHeight - (y + pg->position.y + bmglyph->top) ;

	 drawGlyphInARGBImage(true,
					  src, sWidth, sHeight,
					  red, green, blue,
					  dst, dWidth, dHeight,
					  dX, dY) ;
    }

  }

  void
  glString::getAsImage(Image *image,
				   unsigned char red, unsigned char green, unsigned char blue,
				   unsigned int border) {
    if (pglyphs.empty()) return ;

    float xmin, ymin, xmax, ymax ;
    bbox(&xmin, &ymin, &xmax, &ymax) ;
    // std::cerr << "bbox: " << xmin << "," << ymin << " - " << xmax << "," << ymax << std::endl ;

    unsigned dWidth = (unsigned)(xmax-xmin) + 2*border ;
    unsigned dHeight = (unsigned)(ymax-ymin) + 2*border ;
    if (dHeight%2) dHeight++ ;
    image->prepareFor(dWidth, dHeight, Image::ARGB) ;
    unsigned char *dst = image->getData() ;

    for (std::list<pGlyph*>::const_iterator i=pglyphs.begin();
	    i!=pglyphs.end();
	    ++i) {
	 pGlyph *pg = (*i) ;
	 FT_BitmapGlyph bmglyph = (FT_BitmapGlyph)pg->glyph ;

	 if ((!bmglyph->bitmap.width) || (!bmglyph->bitmap.rows)) continue ;

	 unsigned char *src = bmglyph->bitmap.buffer ;
	 unsigned sWidth = bmglyph->bitmap.width ;
	 unsigned sHeight = bmglyph->bitmap.rows ;

	 int dX = (int)(border - xmin + pg->position.x + bmglyph->left) ;
	 int dY = (int)(border + ymax - pg->position.y - bmglyph->top) ;

	 drawGlyphInARGBImage(false,
					  src, sWidth, sHeight,
					  red, green, blue,
					  dst, dWidth, dHeight,
					  dX, dY) ;
    }

  }

  // ------------------------------------------------------------------------

  glString&
  glString::operator<< (int i) {
    std::stringstream tmp ;
    tmp << i ;
    std::string stmp = tmp.str() ;
    append(stmp.c_str(), stmp.size()) ;
    return *this ;
  }

  glString&
  glString::operator<< (unsigned int i) {
    std::stringstream tmp ;
    tmp << i ;
    std::string stmp = tmp.str() ;
    append(stmp.c_str(), stmp.size()) ;
    return *this ;
  }

  glString&
  glString::operator<< (long l) {
    std::stringstream tmp ;
    tmp << l ;
    std::string stmp = tmp.str() ;
    append(stmp.c_str(), stmp.size()) ;
    return *this ;
  }

  glString&
  glString::operator<< (float f) {
    std::stringstream tmp ;
    tmp << f ;
    std::string stmp = tmp.str() ;
    append(stmp.c_str(), stmp.size()) ;
    return *this ;
  }

  glString&
  glString::operator<< (double d) {
    std::stringstream tmp ;
    tmp << d ;
    std::string stmp = tmp.str() ;
    append(stmp.c_str(), stmp.size()) ;
    return *this ;
  }
 
}
