/*
 *
 * nucleo/image/processing/basic/Resize.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/processing/basic/Resize.H>
#include <nucleo/image/encoding/Conversion.H>

#include <cstring>
#include <cstdlib>

#ifdef __APPLE__
#include <Accelerate/Accelerate.h>
#endif

namespace nucleo {

  // ------------------------------------------------------------------

  static bool
  doResize(unsigned char *source, const unsigned width, const unsigned height,
		 unsigned char *target, const unsigned newWidth, const unsigned newHeight,
		 const unsigned int bpp) {
    if (!target || !source || !width || !height || !newWidth || !newHeight) return false ;

    int *x_src_offsets = new int [newWidth*bpp] ;
    int *y_src_offsets = new int [newHeight] ;
  
    //  pre-calc the scale tables
    unsigned int x,y,b ;
    for (b = 0; b < bpp; b++) {
	 for (x = 0; x < newWidth; x++) {
	   x_src_offsets [b + x * bpp] = b + bpp * ((x * width + width / 2) / newWidth) ;
	 }
    }
    for (y = 0; y < newHeight; y++) {
	 y_src_offsets [y] = (y * height + height / 2) / newHeight ;
    }

    unsigned int row_bytes = newWidth * bpp ;
    unsigned char *dest = new unsigned char [row_bytes] ;
    int last_src_y = -1 ;
    for (y = 0; y < newHeight; y++) {
	 int yoffset = y_src_offsets[y] ;
	 // if the source of this line was the same as the source
	 // of the last line, there's no point in re-rescaling.
	 if (yoffset != last_src_y) {
	   unsigned char *src = source + yoffset*width*bpp ;
	   for (x = 0; x < row_bytes ; x++)  dest[x] = src[x_src_offsets[x]] ;
	   last_src_y = yoffset ;
	 }
	 memmove(target+y*row_bytes,dest,row_bytes) ;
    }
  
    delete [] x_src_offsets ;
    delete [] y_src_offsets ;
    delete [] dest ;
    return true ;
  }

  // ------------------------------------------------------------------

  static bool
  resizeYpCbCr420Image(Image *img, unsigned int nw, unsigned int nh) {
    unsigned int w = img->getWidth() ;
    unsigned int h = img->getHeight() ;
    if (!w || !h || (w==nw && h==nh)) return false ;

    unsigned int size = w*h ;

    unsigned char *srcYp = img->getData() ;
    unsigned char *srcCb = srcYp + size ; 
    unsigned char *srcCr = srcYp + size + size/4 ; 

    unsigned int nsize = nw*nh ;
    unsigned char *ndata = (unsigned char *)Image::AllocMem((unsigned int)(nsize*1.5)) ;
    unsigned char *dstYp = ndata ;
    unsigned char *dstCb = dstYp + nsize ; 
    unsigned char *dstCr = dstYp + (int)(nsize*1.25) ; 

    if (doResize(srcYp,w,h, dstYp,nw,nh, 1)
	   && doResize(srcCb,w,h, dstCb,nw/4,nh/4, 1)
	   && doResize(srcCr,w,h, dstCr,nw/4,nh/4, 1)) {
	 img->setDims(nw, nh) ;
	 img->setData(ndata, nsize, Image::FREEMEM) ;
	 return true ;
    }

    Image::FreeMem((unsigned char **)&ndata) ;
    return false ;
  }

  // ------------------------------------------------------------------

  bool
  resizeImage(Image *img, unsigned int width, unsigned int height) {
    unsigned int oldWidth = img->getWidth() ;
    unsigned int oldHeight = img->getHeight() ;

    if (!oldWidth || !oldHeight
	   || !width || !height
	   || (oldWidth==width && oldHeight==height))
	 return false ;

    if (img->getEncoding()==Image::YpCbCr420)
	 return resizeYpCbCr420Image(img, width, height) ;    

    if (!convertImage(img, Image::CONVENIENT)) return false ;

    unsigned int bytesPerPixel = img->getBytesPerPixel() ;
    unsigned int newSize = (int)(width*height*bytesPerPixel) ;
    unsigned char *newData = (unsigned char *)Image::AllocMem(newSize) ;

#if __APPLE__
    if (img->getEncoding()==Image::ARGB) {
	 vImage_Buffer visrc = {img->getData(),oldHeight,oldWidth,oldWidth*4} ;
	 vImage_Buffer vidst = {newData,height,width,width*4} ;
	 vImage_Error error = vImageScale_ARGB8888(&visrc, &vidst, NULL,
									   kvImageNoFlags/*kvImageHighQualityResampling*/) ;
	 if (error==kvImageNoError) {
	   img->setDims(width, height) ;
	   img->setData(newData, newSize, Image::FREEMEM) ;
	   return true ;
	 }
    }
#endif

    if (doResize((unsigned char *)img->getData(), oldWidth, oldHeight,
			  newData, width, height,
			  bytesPerPixel)) {
	 img->setDims(width, height) ;
	 img->setData(newData, newSize, Image::FREEMEM) ;
	 return true ;
    }

    Image::FreeMem((unsigned char **)&newData) ;
    return false ;
  }

  // ------------------------------------------------------------------

  bool
  resizeImage(Image *src, Image *dst, unsigned int width, unsigned int height) {
    unsigned int oldWidth = src->getWidth() ;
    unsigned int oldHeight = src->getHeight() ;
    if (!oldWidth || !oldHeight || !width || !height) return false ;

    if (oldWidth==width && oldHeight==height) {
	 dst->linkDataFrom(*src) ;
	 return true ;
    }

    Image tmp(*src) ;

    if (!convertImage(&tmp, Image::CONVENIENT)) return false ;

    unsigned int bytesPerPixel = tmp.getBytesPerPixel() ;

    unsigned int size = width*height*bytesPerPixel ;
    if (dst->getSize()!=size)
	 dst->setData(Image::AllocMem(size), size, Image::FREEMEM) ;
    dst->setEncoding(tmp.getEncoding()) ;
    dst->setDims(width,height) ;

    return doResize((unsigned char *)tmp.getData(), tmp.getWidth(), tmp.getHeight(),
				(unsigned char *)dst->getData(), width, height,
				bytesPerPixel) ;
  }

  // ------------------------------------------------------------------

  bool
  cropImage(Image *img,
		  unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
    Image tmp ;
    if (!cropImage(img, x1,y1,x2,y2, &tmp)) return false ;
    img->stealDataFrom(tmp) ;
    return true ;
  }

  bool
  cropImage(Image *src,
		  unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2,
		  Image *dst) {
    unsigned int oldWidth = src->getWidth() ;
    unsigned int oldHeight = src->getHeight() ;

    if (x1>=oldWidth || x2>=oldWidth || y1>=oldHeight || y2>=oldHeight) return false ;

    int newWidth = x2-x1+1 ;
    int newHeight = y2-y1+1 ;
    if (newWidth<=0 || newHeight<=0) return false ;

    Image tmp(*src) ;
    if (!convertImage(&tmp, Image::CONVENIENT)) return false ;

    if (newWidth==(int)tmp.getWidth() && newHeight==(int)tmp.getHeight()) {
	 // dst->copyDataFrom(tmp) ;
	 dst->linkDataFrom(tmp) ;
    } else {
	 dst->prepareFor(newWidth,newHeight,tmp.getEncoding()) ;
	 unsigned int bytesPerPixel = dst->getBytesPerPixel() ;
	 unsigned char *ptrSrc = tmp.getData() + (y1*oldWidth + x1)*bytesPerPixel ;
	 unsigned char *ptrDst = dst->getData() ;
	 unsigned int dstBlock = newWidth*bytesPerPixel ;
	 unsigned int srcBlock = oldWidth*bytesPerPixel ;
	 for (int i=newHeight; i>0; i--) {
	   memmove(ptrDst, ptrSrc, dstBlock) ;
	   ptrDst += dstBlock ;
	   ptrSrc += srcBlock ;
	 }
    }

    return true ;
  }

  // ------------------------------------------------------------------

  ResizeFilter::ResizeFilter(const char *geometry) {
    width = height = 0 ;
    width = atoi(geometry) ;
    int i=0 ; 
    while (geometry[i]!='\0'&&geometry[i]!='x') ++i ;
    if (geometry[i]=='x') height = atoi(geometry+i+1) ;
  }

  // ------------------------------------------------------------------

  CropFilter::CropFilter(const char *str) {
    x1 = y1 = x2 = y2 = 0 ;
    char *ptr = (char *)str ;
    x1 = atoi(ptr) ;
    for (;*ptr!=',';++ptr) if (*ptr=='\0') return ;
    ptr++ ;
    y1 = atoi(ptr) ;
    for (;*ptr!='-';++ptr) if (*ptr=='\0') return ;
    ptr++ ;
    x2 = atoi(ptr) ;
    for (;*ptr!=',';++ptr) if (*ptr=='\0') return ;
    ptr++ ;
    y2 = atoi(ptr) ;
    std::cerr << "CropFilter: " << x1 << "," << y1 << " - " << x2 << "," << y2 << std::endl ;
  }

  void
  maxResize(float iw, float ih, float maxw, float maxh, float *ow, float *oh) {
    float zoom = maxw/iw ;
    (*ow) = maxw ;
    (*oh) = zoom*ih ;
    if (*oh>maxh) {
	 zoom = maxh/(*oh) ;
	 (*ow) = (*ow)*zoom ;
	 (*oh) = maxh ;
    }
  }

  // ------------------------------------------------------------------
  // Below is some unused old stuff from videoSpace...
  // ------------------------------------------------------------------

#if 0

  float
  computeMaxZoomFactor(float iw, float ih, float ow, float oh) {
    float zoom ;
    zoom = ow/iw ;
    float h=ih*zoom ;
    if (h>oh) zoom = zoom*(oh/h) ;
    return zoom ;
  }
#endif

  // ------------------------------------------------------------------

}
