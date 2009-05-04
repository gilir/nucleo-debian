#ifndef n_Resize_H
#define n_Resize_H

/*
 *
 * nucleo/image/processing/basic/Resize.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/processing/ImageFilter.H>

namespace nucleo {

  // ------------------------------------------------------------------

  // img will be converted to a "convenient" encoding if it's not and
  // then resized "in place"
  bool resizeImage(Image *img, unsigned int width, unsigned int height) ;

  // dst will be src resized to width x height pixels. dst will be in
  // a "convenient" encoding, src won't be changed
  bool resizeImage(Image *src,
			    Image *dst, unsigned int width, unsigned int height) ;

  // img will be converted to a "convenient" encoding if it's not and
  // then cropped "in place"
  bool cropImage(Image *img,
			  unsigned int x1, unsigned int y1,
			  unsigned int x2, unsigned int y2) ;

  bool cropImage(Image *src,
			  unsigned int x1, unsigned int y1,
			  unsigned int x2, unsigned int y2,
			  Image *dst) ;

  void maxResize(float iw, float ih,
			  float maxw, float maxh,
			  float *ow, float *oh) ;

  // ------------------------------------------------------------------

  class ResizeFilter : public ImageFilter {
  private:
    unsigned int width, height ;
  public:
    ResizeFilter(unsigned int w, unsigned int h) : width(w), height(h) {}
    ResizeFilter(const char *geometry) ; // e.g. "400x300"
    bool filter(Image *img) { return resizeImage(img, width, height) ; }
  } ;

  class CropFilter : public ImageFilter {
  private:
    unsigned int x1, y1, x2, y2 ;
  public:
    CropFilter(unsigned int a, unsigned int b,
			unsigned int c, unsigned int d) : x1(a), y1(b), x2(c), y2(d) {}
    CropFilter(const char *str) ; // e.g. "10,10-20-20"
    bool filter(Image *img) { return cropImage(img, x1, y1, x2, y2) ; }
  } ;

  // ------------------------------------------------------------------

}

#endif
