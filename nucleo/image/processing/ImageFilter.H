#ifndef n_ImageFilter_H
#define n_ImageFilter_H

/*
 *
 * nucleo/image/processing/Imagefilter.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/Image.H>

namespace nucleo {

  class ImageFilter {
  public:
    virtual ~ImageFilter(void) {}
    virtual bool filter(Image *img)=0 ;
  } ;

}

#endif
