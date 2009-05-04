/*
 *
 * nucleo/image/processing/difference/Difference.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/processing/difference/Difference.H>
#include <nucleo/image/encoding/Conversion.H>

#include <cmath>
#include <cstring>

#include <stdexcept>

namespace nucleo {

  // ----------------------------------------------------------------------------

  static inline double
  luminosity(unsigned char *pixel, Image::Encoding e) {
    double r=0.0, g=0.0, b=0.0 ;
    switch (e) {
    case Image::A:
    case Image::L:
	 return (double)pixel[0] ;
    case Image::RGB:
	 r = pixel[0] ; g = pixel[1] ; b = pixel[2] ;
	 break ;
    case Image::ARGB:
	 r = pixel[1] ; g = pixel[2] ; b = pixel[3] ;
	 break ;
    case Image::RGBA:
	 r = pixel[0] ; g = pixel[1] ; b = pixel[2] ;
	 break ;
    default:
	 throw std::runtime_error("Difference(luminosity): bad image encoding") ;
    } 
    return 0.11*b + 0.59*g + 0.3*r ;
  }

  // ----------------------------------------------------------------------------

  bool
  DifferenceFilter::filter(Image *img) {
    if (!convertImage(img, Image::CONVENIENT)) return false ;

    const unsigned int w = img->getWidth() ;
    const unsigned int h = img->getHeight() ;

    Image::Encoding encoding = img->getEncoding() ;
    if (encoding!=_lastimage.getEncoding()
	   || w!=_lastimage.getWidth()
	   || h!=_lastimage.getHeight()) {
	 _lastimage.copyDataFrom(*img) ;
	 return true ;
    }

    unsigned char *newData=(unsigned char *)img->getData() ;
    const unsigned int size = img->getSize() ;
    unsigned char *ptr=0 ;
    if (!frozen) {
	 ptr = (unsigned char *)Image::AllocMem(size) ;
	 memmove(ptr, newData, size) ;
    }

    unsigned char *oldData=(unsigned char *)_lastimage.getData() ;
    const int bpp = img->getBytesPerPixel() ;
    const unsigned int nbpixels=w*h ;
    for (unsigned int i=0; i<nbpixels; ++i) {
	 double vOld = luminosity(oldData, encoding) ; oldData+=bpp ;
	 double vNew = luminosity(newData, encoding) ; 
	 if (fabs(vOld-vNew)<threshold)
	   memset((void *)newData, 0, bpp) ;
	 newData+=bpp ;
    }

    if (!frozen) _lastimage.setData(ptr, size, Image::FREEMEM) ;

    return true ;
  }

  // ----------------------------------------------------------------------------

  bool
  DifferencePattern::filter(Image *img, bool changeImage) {
	 if (!convertImage(img, Image::CONVENIENT)) return false ;

    for (unsigned int i=0; i<_size; ++i) _pattern[i] = 0 ;

    const unsigned int w = img->getWidth() ;
    const unsigned int h = img->getHeight() ;

    Image::Encoding encoding = img->getEncoding() ;
    if (encoding!=_lastimage.getEncoding()
	   || w!=_lastimage.getWidth()
	   || h!=_lastimage.getHeight()) {
	 _lastimage.copyDataFrom(*img) ;
	 return true ;
    }

    unsigned char *newData=(unsigned char *)img->getData() ;
    const unsigned int size = img->getSize() ;
    unsigned char *ptr=0 ;
    if (!frozen) {
	 ptr = (unsigned char *)Image::AllocMem(size) ;
	 memmove(ptr, newData, size) ;
    }

    unsigned char *oldData=(unsigned char *)_lastimage.getData() ;
    const int bpp = img->getBytesPerPixel() ;
    const unsigned int nbpixels=w*h ;
    double pixelCnt = 100.0 * _size / nbpixels ;
    for (unsigned int i=0; i<nbpixels; ++i) {
	 double vOld = luminosity(oldData, encoding) ; oldData+=bpp ;
	 double vNew = luminosity(newData, encoding) ;
	 if ( fabs(vOld-vNew) >= threshold ) {
	   int c = (i%w)*_width/w ;
	   int l = (i/w)*_height/h ;
	   _pattern[c+l*_width] += pixelCnt ;
	 } else {
	   if (changeImage) memset((void *)newData, 0, bpp) ;
	 }
	 newData+=bpp ;
    }

    if (!frozen) _lastimage.setData(ptr, size, Image::FREEMEM) ;
    return true ;
  }

  void
  DifferencePattern::debug(std::ostream& out) {
    out << "reference image" ;
    if (frozen) out << " (frozen)" ;
    out << ": " ; _lastimage.debug(out) ; out << std::endl ;
    for (unsigned int l=0; l<_height; ++l) {
	 for (unsigned int c=0; c<_width; ++c)
	   out << (int)_pattern[l*_width+c] << " " ;
	 out << std::endl ;
    }
  }

  // ----------------------------------------------------------------------------

}
