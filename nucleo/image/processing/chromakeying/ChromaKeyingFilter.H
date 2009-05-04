#ifndef n_ChromaKeying_H
#define n_ChromaKeying_H

/*
 *
 * nucleo/image/processing/chromakeying/ChromaKeying.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/processing/ImageFilter.H>

namespace nucleo {

  class ChromaKeyingFilter : public ImageFilter {

  private:

  public:

    typedef enum {RED, GREEN, BLUE} ColorIndex ;

    int key[3] ;
    float threshold, blackThreshold ;
    unsigned char transparency ;

    ChromaKeyingFilter(int red=0, int green=0, int blue=0, float t=0.01, float bt=0.0) {
	 transparency = 255 ;
	 setKey(red, green, blue, t, bt) ;
    }

    void setKey(int red, int green, int blue, float t=-1.0, float bt=-1.0) {
	 key[RED] = red ;
	 key[GREEN] = green ;
	 key[BLUE] = blue ;
	 if (t>=0.0) threshold = t ;
	 if (bt>=0.0) blackThreshold = bt ;
    }

    bool getKey(Image *img) ; // img MUST be ARGB
    bool filter(Image *img) ; // img MUST be ARGB

    void debug(void) {
	 std::cerr << "Key : R=" << key[RED] << " G=" << key[GREEN] << " B=" << key[BLUE] << std::endl ;
	 std::cerr << "Threshold : " << threshold << std::endl ;
	 std::cerr << "Black threshold : " << blackThreshold << std::endl ;
    }

  } ;

}

#endif
