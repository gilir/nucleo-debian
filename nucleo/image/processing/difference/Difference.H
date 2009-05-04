#ifndef n_Difference_H
#define n_Difference_H

/*
 *
 * nucleo/image/processing/difference/Difference.H --
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

  class DifferenceFilter : public ImageFilter {

  private:

    Image _lastimage ;

  public:

    unsigned char threshold ;
    bool frozen ;

    DifferenceFilter(unsigned char t=25, bool f=false) : threshold(t), frozen(f) {}

    // img will be converted to a "convenient" encoding if it's not
    bool filter(Image *img) ;

  } ;

  // ------------------------------------------------------------------

  class DifferencePattern : public ImageFilter {

  private:

    Image _lastimage ;
    float *_pattern ;
    unsigned int _width, _height, _size ;

  public:

    unsigned char threshold ;
    bool frozen ;

    DifferencePattern(int width, int height, unsigned char t=25, bool f=false) {
	 _width = width ;
	 _height = height ;
	 threshold = t ;
	 frozen = f ;
	 _size = _width*_height ;
	 _pattern = new float [_size] ;
    }

    ~DifferencePattern(void) {
	 delete [] _pattern ;
    }

    void reset() {
	 _lastimage.clear() ;
	 for (unsigned int i=0; i<_size; i++) _pattern[i] = 0 ;
    }

    bool filter(Image *img) { return filter(img,false) ; }

    // img will be converted to a "convenient" encoding if it's
    // not. resets pattern to 0 before applying the filter
    bool filter(Image *img, bool changeImage) ;

    float *getPattern(void) { return _pattern ; }
    unsigned int getWidth(void) { return _width ; }
    unsigned int getHeight(void) { return _height ; }

    void debug(std::ostream& out) ;
  } ;

  // ------------------------------------------------------------------

}

#endif
