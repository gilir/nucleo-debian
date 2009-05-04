#ifndef n_Convolution_H
#define n_Convolution_H

/*
 *
 * nucleo/image/processing/convolution/Convolution.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/processing/ImageFilter.H>

namespace nucleo {

  class Convolution_3x3 : public ImageFilter {

  private:

    double _filter[9] ;
    double _add ;
    double _divide ;

  public:

    Convolution_3x3(double f11, double f12, double f13,
				double f21, double f22, double f23,
				double f31, double f32, double f33,
				double a=0, double d=1) {
	 _filter[0] = f11 ; _filter[1] = f12 ; _filter[2] = f13 ;
	 _filter[3] = f21 ; _filter[4] = f22 ; _filter[5] = f23 ;
	 _filter[6] = f31 ; _filter[7] = f32 ; _filter[8] = f33 ;
	 _add = a ; _divide = d ;
    }

    // img will be converted to a "convenient" encoding if it's not
    bool filter(Image *img) ;

  } ;

}

#endif
