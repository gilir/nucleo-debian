/*
 *
 * nucleo/image/processing/convolution/Convolution.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/processing/convolution/Convolution.H>
#include <nucleo/image/encoding/Conversion.H>

namespace nucleo {

  bool
  Convolution_3x3::filter(Image *img) {
    if (!convertImage(img, Image::CONVENIENT)) return false ;
    
    unsigned int bpp = img->getBytesPerPixel() ;
    unsigned char *data = (unsigned char *)img->getData() ;
    unsigned int width = img->getWidth() ;
    unsigned int height = img->getHeight() ;
    unsigned int size = img->getSize() ;

    unsigned char *buffer = (unsigned char *)Image::AllocMem(size) ;

    unsigned int realwidth = width*bpp ;

    for (unsigned int l=1; l<(height-1); ++l) {
	 unsigned int loffset = l*width*bpp ;
	 for (unsigned int c=1; c<(width-1); ++c) {
	   register unsigned int nw, n, ne, w, e, sw, s, se ;
	   register unsigned int pixel = loffset + c*bpp ;

	   w = pixel-bpp ;
	   e = pixel+bpp ;
	   n = pixel - realwidth ;
	   nw = n-bpp ; ne = n+bpp ;
	   s = pixel + realwidth ;
	   sw = s-bpp ; se = s+bpp ;

	   for (unsigned int channel=0; channel<bpp; ++channel){
		double v = _add + (
					   _filter[0] * data[nw++] + _filter[1] * data[n++] + _filter[2] * data[ne++]
					   + _filter[3] * data[w++] + _filter[4] * data[pixel] + _filter[5] * data[e++]
					   + _filter[6] * data[sw++] + _filter[7] * data[s++] + _filter[8] * data[se++]
					   ) / _divide ;
		if (v<0) v=0 ; else if (v>255) v=255 ;
		buffer[pixel++] = (unsigned char)v  ;
	   }
	 }
    }

    img->setData(buffer, size, Image::FREEMEM) ;

    return true ;
  }

}
