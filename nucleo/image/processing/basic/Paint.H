#ifndef n_Paint_H
#define n_Paint_H

/*
 *
 * nucleo/image/processing/basic/Paint.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/Image.H>
#include <nucleo/image/processing/ImageFilter.H>

#include <cstdlib>

namespace nucleo {

  // ------------------------------------------------------------------

  bool parseColorCode(std::string color, uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *alpha) ;

  // ------------------------------------------------------------------

  // Simple midpoint algorithm, no clipping. Lines that are not
  // entirely in the image will not be drawn.
  bool
  drawLine(Image *img,
		 unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2,
		 unsigned char R, unsigned char G, unsigned char B, unsigned char A) ;

  bool
  drawRectangle(Image *img,
			 unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2,
			 unsigned char R, unsigned char G, unsigned char B, unsigned char A) ;

  // ------------------------------------------------------------------

  // img will be converted to a "convenient" encoding if it's not
  bool paintImageRegion(Image *img,
				    unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2,
				    unsigned char R, unsigned char G, unsigned char B, unsigned char A) ;

  // img will be converted to a "convenient" encoding if it's not
  bool paintImage(Image *img,
			   unsigned char R, unsigned char G, unsigned char B, unsigned char A) ;

  // ------------------------------------------------------------------

  // dst will be converted to a "convenient" encoding if it's not
  bool drawImageInImage(Image *src, Image *dst, int x, int y) ;

  // ------------------------------------------------------------------

  // src1 and src2 must have the same width and height. If they don't
  // have the same encoding or if it's not a "convenient" one, dst
  // will be ARGB. For the blending, 0.0 is src1 only, 1.0 is src2
  // only
  bool blendImages(Image *src1, Image *src2, Image *dst, float blending) ; 

  // src1 and src2 must have the same width and height. src2 must be
  // ARGB. dst will have the same encoding as src1, except if it's not
  // ARGB, RGB or L (in that case, dst will be ARGB). The alpha
  // channel of src2 is used for blending
  bool blendImages(Image *src1, Image *src2, Image *dst) ;

  // ------------------------------------------------------------------

}

#endif
