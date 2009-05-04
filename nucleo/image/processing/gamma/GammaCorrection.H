#ifndef n_GammaCorrection_H
#define n_GammaCorrection_H

/*
 *
 * nucleo/processing/gamma/GammaCorrection.H --
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

  class GammaByteCorrection : public ImageFilter {

  private:

    unsigned char _table[256] ;
    double _value ;

    void _mktable(double v) ;

  public:

    GammaByteCorrection(double v=1.0) { _mktable(v) ; }

    // img will be converted to a "convenient" encoding if it's not 
    bool filter(Image *img) ;

    double getCorrectionValue(void) { return _value ; }
    void setCorrectionValue(double v) { if (v!=_value) _mktable(v) ; }
    
  } ;

  // ------------------------------------------------------------------

}

#endif
