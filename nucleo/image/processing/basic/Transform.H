#ifndef n_Transform_H
#define n_Transform_H

/*
 *
 * nucleo/image/processing/basic/Transform.H --
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
  
  // 90 degrees rotation. img will be converted to a "convenient"
  // encoding if it's not
  bool rotateImage(Image *img, bool clockwise) ;

  // Use 'h' for horizontal (columns are moved, like a real mirror) or
  // 'v' for vertical (lines are moved). img will be converted to a
  // "convenient" encoding if it's not
  bool mirrorImage(Image *img, char hORv) ; 

  // ------------------------------------------------------------------

  class rotateFilter : public ImageFilter {
  private:
    bool clockwise ;
  public:
    rotateFilter(bool cw) : clockwise(cw) {}
    bool filter(Image *img) { return rotateImage(img, clockwise) ; }
  } ;

  class mirrorFilter : public ImageFilter {
  private:
    char hORv ;
  public:
    mirrorFilter(char mode) : hORv(mode) {}
    bool filter(Image *img) { return mirrorImage(img,hORv) ; }
  } ;

  // ------------------------------------------------------------------

}

#endif
