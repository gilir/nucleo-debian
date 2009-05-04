/*
 *
 * nucleo/image/processing/convolution/Blur.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/processing/convolution/Blur.H>

#include <nucleo/image/encoding/Conversion.H>

namespace nucleo {

  bool
  BlurFilter::filter(Image *img,
				 bType type, unsigned int size, unsigned int repeat) {
    if (!repeat || !size) return true ;

    if (!convertImage(img, Image::CONVENIENT)) return false ;
    if (img->dataIsLinked()) img->acquireData() ;

    const unsigned int width = img->getWidth() ;
    const unsigned int height = img->getHeight() ;
    const unsigned int bytesPerPixel = img->getBytesPerPixel() ;

    // std::cerr << "Blur: original size is " << size << std::endl ;
    if (width<2*size+1) size = (width/2) - 1 ;
    if (height<2*size+1) size = (height/2) - 1 ;
    // std::cerr << "Blur: size is now " << size << std::endl ;

    int *accums = new int [bytesPerPixel] ;

    Image tmp(width,height,img->getEncoding()) ;

    for (unsigned int r=0; r<repeat; ++r) {
	 switch (type) {
	 case HandV:
	   horizontalBlur(img, &tmp, size, accums) ;
	   verticalBlur(&tmp, img, size, accums) ;
	   break ;
	 case H:
	   horizontalBlur(img, &tmp, size, accums) ;
	   img->stealDataFrom(tmp) ;
	   break ;
	 case V:
	   verticalBlur(img, &tmp, size, accums) ;
	   img->stealDataFrom(tmp) ;
	   break ;
	 }
    }

    delete [] accums ;
    return true ;
  }

  void
  BlurFilter::horizontalBlur(Image *src, Image *dst, unsigned int size, int *accums) {
    const unsigned int bpp = src->getBytesPerPixel() ;
    const unsigned int width = src->getWidth() ;
    const unsigned int height = src->getHeight() ;
    const unsigned int rowsize = width*bpp ;
    const unsigned int boxsize = 2*size+1 ;

    // 0123456789
    // pxx-------
    // xpxx------
    // xxpxx-----
    // -xxpxx----
    // ..........
    // -----xxpxx
    // ------xxpx
    // -------xxp

    for (unsigned int line=0; line<height; ++line) {
	 unsigned char *sptr = (unsigned char *)(src->getData() + line*rowsize) ;
	 unsigned char *dptr = (unsigned char *)(dst->getData() + line*rowsize) ;
	 unsigned int pixel = 0 ;

	 // Load the first pixel in the accumulators
	 for (unsigned int comp=0; comp<bpp; ++comp)
	   accums[comp] = *(sptr+comp) ;
	 sptr += bpp ;
	 
	 // Add the next (size) pixels to the accumulators
	 for (unsigned int i=0; i<size; ++i) {
	   for (unsigned int comp=0; comp<bpp; ++comp) {
		accums[comp] += *(sptr+comp) ;
	   }
	   sptr += bpp ;
	 }

	 int weightsum = size+1 ;

	 // Set the first (size) pixels of dst image (no left context)
	 for (;pixel<size;++pixel) {
	   for (unsigned int comp=0; comp<bpp; ++comp) {
		*(dptr+comp) = (unsigned char)(accums[comp]/weightsum) ;
		accums[comp] += *(sptr+comp) ;
	   }
	   dptr += bpp ;
	   sptr += bpp ;
	   weightsum++ ;
	 }

	 // Move along the "normal" pixels (left and right context)
	 // -1 was added to avoid a sig fault, however I'm not sure that's correct
	 for (; pixel<width-size-1; ++pixel) {
	   for (unsigned int comp=0; comp<bpp; ++comp) {
		*(dptr+comp) = (unsigned char)(accums[comp]/weightsum) ;
		accums[comp] -= *(sptr-boxsize*bpp+comp) ;
		accums[comp] += *(sptr+comp) ;
	   }
	   dptr += bpp ;
	   sptr += bpp ;
	 }

	 // Set the (size) last pixels of dst image (no right context)
	 for (; pixel<width; ++pixel) {
	   for (unsigned int comp=0; comp<bpp; ++comp) {
		*(dptr+comp) = (unsigned char)(accums[comp]/weightsum) ;
		accums[comp] -= *(sptr-boxsize*bpp+comp) ;
	   }
	   dptr += bpp ;
	   sptr += bpp ;
	   weightsum-- ;
	 }
    }
  }

  void
  BlurFilter::verticalBlur(Image *src, Image *dst, unsigned int size, int *accums) {
    const unsigned int bpp = src->getBytesPerPixel() ;
    const unsigned int width = src->getWidth() ;
    const unsigned int height = src->getHeight() ;
    const unsigned int rowsize = width*bpp ;
    const unsigned int boxsize = 2*size+1 ;

    for (unsigned int column=0; column<width; ++column) {
	 unsigned char *sptr = (unsigned char *)(src->getData() + column*bpp) ;
	 unsigned char *dptr = (unsigned char *)(dst->getData() + column*bpp) ;
	 unsigned int pixel = 0 ;

	 // Load the first pixel in the accumulators
	 for (unsigned int comp=0; comp<bpp; ++comp)
	   accums[comp] = *(sptr+comp) ;
	 sptr += rowsize ;

	 // Add the next (size) pixels to the accumulators
	 for (unsigned int i=0; i<size; ++i) {
	   for (unsigned int comp=0; comp<bpp; ++comp) {
		accums[comp] += *(sptr+comp) ;
	   }
	   sptr += rowsize ;
	 }

	 int weightsum = size+1 ;

	 // Set the first (size) pixels of dst image (no up context)
	 for (;pixel<size;++pixel) {
	   for (unsigned int comp=0; comp<bpp; ++comp) {
		*(dptr+comp) = (unsigned char)(accums[comp]/weightsum) ;
		accums[comp] += *(sptr+comp) ;
	   }
	   dptr += rowsize ;
	   sptr += rowsize ;
	   weightsum++ ;
	 }

#if 0
	 std::cerr << std::endl << "dims = " << src->getWidth() << "x" << src->getHeight() << " " << dst->getWidth() << "x" << dst->getHeight() << " size=" << size << std::endl ;
#endif
	 // Move along the "normal" pixels (up and down context)
	 // -1 was added to avoid a sig fault, however I'm not sure that's correct
	 for (; pixel<height-size-1; ++pixel) {
#if 0
	   std::cerr << "[" << (unsigned int)(sptr-src->getData()) << " " ;
	   std::cerr << (unsigned int)(dptr-dst->getData()) << "] " ;
#endif
	   for (unsigned int comp=0; comp<bpp; ++comp) {
		*(dptr+comp) = (unsigned char)(accums[comp]/weightsum) ;
		accums[comp] -= *(sptr+comp-boxsize*rowsize) ;
		accums[comp] += *(sptr+comp) ;
	   }
	   dptr += rowsize ;
	   sptr += rowsize ;
	 }
#if 0
	 std::cerr << std::endl ;
#endif

	 // Set the (size) last pixels of dst image (no down context)
	 for (; pixel<height; ++pixel) {
	   for (unsigned int comp=0; comp<bpp; ++comp) {
		*(dptr+comp) = (unsigned char)(accums[comp]/weightsum) ;
		accums[comp] -= *(sptr-boxsize*rowsize+comp) ;
	   }
	   dptr += rowsize ;
	   sptr += rowsize ;
	   weightsum-- ;
	 }
    }

  }

}
