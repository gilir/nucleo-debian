/*
 *
 * nucleo/image/processing/basic/Transform.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/processing/basic/Transform.H>

#include <nucleo/image/encoding/Conversion.H>

#include <cstring>

#ifdef __APPLE__
#include <Accelerate/Accelerate.h>
#endif

namespace nucleo {

  // ------------------------------------------------------------------

  bool
  rotateImage(Image *img, bool clockwise) {
    unsigned int width=img->getWidth(), height=img->getHeight() ;
    if (!width || !height) return false ;

    if (!convertImage(img, Image::CONVENIENT)) return false ;

    int size = img->getSize() ;
    unsigned char *new_ptr = Image::AllocMem(size) ;
    unsigned char *old_ptr = img->getData() ;

#ifdef __APPLE__
    if (img->getEncoding()==Image::ARGB) {
	 vImage_Buffer visrc = {old_ptr,height,width,width*4} ;
	 vImage_Buffer vidst = {new_ptr,width,height,height*4} ;
	 Pixel_8888 backgroundColor = {0,0,0,0} ;
	 uint8_t rc = clockwise ? kRotate90DegreesClockwise : kRotate90DegreesCounterClockwise ;
	 vImage_Error error = vImageRotate90_ARGB8888(&visrc,&vidst,rc,backgroundColor,kvImageNoFlags) ;
	 if (error==kvImageNoError) {
	   img->setData(new_ptr, size, Image::FREEMEM) ;
	   img->setDims(height, width) ;
	   return true ;
	 }
    }
#endif

    unsigned int bytesPerPixel = img->getBytesPerPixel() ;
    int old_linewidth = width*bytesPerPixel ;
    int new_linewidth = height*bytesPerPixel ;

    for (unsigned int y=0; y<height; ++y)
	 for (unsigned int x=0; x<width; ++x) {
	   int newx, newy ;
	   if (clockwise) {
		newx = height-1-y ;
		newy = x ;
	   } else {
		newx = y ;
		newy = width-1-x ;
	   }
	   unsigned char *src = old_ptr + y*old_linewidth + x*bytesPerPixel ;
	   unsigned char *dst = new_ptr + newy*new_linewidth + newx*bytesPerPixel ;
	   memmove(dst, src, bytesPerPixel) ;
	 }

    img->setData(new_ptr, size, Image::FREEMEM) ;
    img->setDims(height, width) ;
    return true ;
  }

  // ------------------------------------------------------------------

  bool
  mirrorImage(Image *img, char hORv) {
    unsigned int width=img->getWidth(), height=img->getHeight() ;
    if (!width || !height) return false ;

    if (!convertImage(img, Image::CONVENIENT)) return false ;

    unsigned int size = img->getSize() ;
    unsigned char *mirror_ptr = Image::AllocMem(size) ;

#ifdef __APPLE__
    if (img->getEncoding()==Image::ARGB) {
	 vImage_Buffer visrc = {img->getData(),height,width,width*4} ;
	 vImage_Buffer vidst = {mirror_ptr,height,width,width*4} ;
	 vImage_Error error = kvImageNoError ;
	 if (hORv=='h')
	   error = vImageHorizontalReflect_ARGB8888(&visrc,&vidst,kvImageNoFlags) ;
	 else if (hORv=='v')
	   error = vImageVerticalReflect_ARGB8888(&visrc,&vidst,kvImageNoFlags) ;
	 if (error==kvImageNoError) {
	   img->setData(mirror_ptr, size, Image::FREEMEM) ;
	   return true ;
	 }
    }
#endif

    unsigned int bytesPerPixel = img->getBytesPerPixel() ;
    unsigned int linewidth = width*bytesPerPixel ;
    if (hORv=='h') {
	 unsigned char *src=img->getData(), *dst=mirror_ptr ;
	 for (unsigned int y=0; y<height; ++y) {
	   src += linewidth ;
	   for (unsigned int x=0; x<width; ++x) {
		src -= bytesPerPixel ;
		memmove(dst, src, bytesPerPixel) ;
		dst += bytesPerPixel ;
	   }
	   src += linewidth ;
	 }
    } else if (hORv=='v') {
	 unsigned char *src=img->getData(), *dst=mirror_ptr+size-linewidth ;
	 for (unsigned int i=0; i<height; ++i) {
	   memmove(dst, src, linewidth) ;
	   src += linewidth ;
	   dst -= linewidth ;
	 }
    }

    img->setData(mirror_ptr, size, Image::FREEMEM) ;
    return true ;
  }

  // ------------------------------------------------------------------

}
