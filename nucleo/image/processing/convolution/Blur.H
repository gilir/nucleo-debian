#ifndef n_Blur_H
#define n_Blur_H

/*
 *
 * nucleo/image/processing/convolution/Blur.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/processing/ImageFilter.H>

namespace nucleo {

class BlurFilter : public ImageFilter {

private:

  static void horizontalBlur(Image *src, Image *dst,
					    unsigned int size,
					    int *accums) ;
  static void verticalBlur(Image *src, Image *dst,
					  unsigned int size,
					  int *accums) ;

public:

  typedef enum {H, V, HandV} bType ;

private:

  bType _type ;
  unsigned int _size ;
  unsigned int _repeat ;

public:

  BlurFilter(void)
    : _type(HandV), _size(3), _repeat(3) {}

  BlurFilter(bType t, unsigned int s, unsigned int r)
    : _type(t), _size(s), _repeat(r) {}

  // img will be converted to a "convenient" encoding if it's not
  static bool filter(Image *src, bType type, unsigned int size, unsigned int repeat) ;

  bool filter(Image *img) {
    return filter(img,_type,_size,_repeat) ;
  }

} ;

}

#endif
