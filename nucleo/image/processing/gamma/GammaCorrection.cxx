/*
 *
 * nucleo/image/processing/GammaCorrection.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/processing/gamma/GammaCorrection.H>
#include <nucleo/image/encoding/Conversion.H>

#include <cmath>

namespace nucleo {

  // ------------------------------------------------------------------

  void
  GammaByteCorrection::_mktable(double v) {
    _value = v ;
    double gamma = 1.0 / v ;
    for(int i=0; i < 256; i++)
	 _table[i] = (unsigned char)(pow(i / 255.0, gamma) * 255.0) ;
  }

  bool
  GammaByteCorrection::filter(Image *img) {
    if (_value==1.0) return true ;

    if (!convertImage(img, Image::CONVENIENT)) return false ;

    Image::Encoding encoding = img->getEncoding() ;
    unsigned char *ptr = (unsigned char *)img->getData() ;

    switch (encoding) {
    case Image::L:
    case Image::A: {
	 const unsigned int size = img->getSize() ;
	 for (unsigned int i=0; i<size; ++i, ++ptr) *ptr = _table[*ptr] ;
    } break ;
    case Image::RGB:
    case Image::RGBA:
    case Image::ARGB: {
	 unsigned int nbPixels = img->getWidth()*img->getHeight() ;
	 unsigned int bpp = img->getBytesPerPixel() ;
	 if (encoding==Image::ARGB) ptr++ ; // skip A
	 for (unsigned int pixel=0; pixel<nbPixels; ++pixel) {
	   for (unsigned int component=0; component<3; ++component)
		*(ptr+component) = _table[*(ptr+component)] ;
	   ptr += bpp ;
	 }
	 if (encoding==Image::RGBA) ptr++ ; // skip A
    } break ;
    default: return false ;
    }

    return true ;
  }

  // ------------------------------------------------------------------

}
