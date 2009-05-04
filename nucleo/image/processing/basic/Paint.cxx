/*
 *
 * nucleo/image/processing/basic/Paint.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/processing/basic/Paint.H>

#include <nucleo/image/encoding/Conversion.H>

#include <cmath>
#include <cstring>

#if defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_4) 
#include <Accelerate/Accelerate.h>
#endif

namespace nucleo {

  // ------------------------------------------------------------------

  bool
  parseColorCode(std::string color,
			  uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *alpha) {
    if (color[0]!='#') return false ;
    std::string r(color, 1,2) ;
    std::string g(color, 3,2) ;
    std::string b(color, 5,2) ;
    std::string a(color, 7,2) ;
    if (red) *red = (uint8_t)strtol(r.c_str(), NULL, 16) ;
    if (green) *green = (uint8_t)strtol(g.c_str(), NULL, 16) ;
    if (blue) *blue = (uint8_t)strtol(b.c_str(), NULL, 16) ;
    if (alpha) *alpha = (uint8_t)strtol(a.c_str(), NULL, 16) ;
    return true ;
  }

  // ------------------------------------------------------------------

  static inline                                  // 0123
  void drawPixel(unsigned char *ptr, unsigned char *argb, Image::Encoding e) {
    switch (e) {
    case Image::A:
	 *ptr = argb[0] ;
	 break ;
    case Image::L:
	 *ptr = (unsigned char) (0.11*argb[3] + 0.59*argb[2] + 0.3*argb[1]) ;
	 break ;
    case Image::RGB:
	 memmove(ptr,argb+1,3) ;
	 break ;
    case Image::ARGB:
	 memmove(ptr,argb,4) ;
	 break ;
    case Image::RGBA:
	 memmove(ptr,argb+1,3) ;
	 ptr[3] = argb[0] ;
	 break ;
    case Image::RGB565:
	 *ptr = (((argb[1] >> 3) & 0x1f) << 11)  |
	   (((argb[2] >> 2) & 0x3f) << 5)   |
	   (((argb[3] >> 3) & 0x1f) << 0) ;
	 break ;
    default:
	 std::cerr << "drawPixel (Paint.cxx): unsupported encoding (" << Image::getEncodingName(e) << ")" << std::endl ;
	 break ;
    }
  }

  bool
  drawLine(Image *img,
		 unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2,
		 unsigned char R, unsigned char G, unsigned char B, unsigned char A) {
    if (!convertImage(img, Image::CONVENIENT)) return false ;

    unsigned int width = img->getWidth() ;
    unsigned int height = img->getHeight() ;
    unsigned int bytesPerPixel = img->getBytesPerPixel() ;
    unsigned char *data = (unsigned char *)img->getData() ;
    int lineWidth = width*bytesPerPixel ;
    Image::Encoding encoding = img->getEncoding() ;
    unsigned char argb[4] = {A, R, G, B} ;

    float dx=(float)x2-(float)x1, dy=(float)y2-(float)y1 ;
    float absdx=fabs(dx), absdy=fabs(dy) ;
    float delta = 1.0 / ((absdx>absdy) ? absdx : absdy) ;

    for (float t=0.0; t<=1.0; t+=delta) {
	 unsigned int x = (unsigned int)(x1 + dx*t) ;
	 unsigned int y = (unsigned int)(y1 + dy*t) ;
	 if (x>=0 && y>=0 && x<width && y<height)
	   drawPixel(data+x*bytesPerPixel+y*lineWidth, argb, encoding) ;
    }

    return true ;
  }

  bool
  drawRectangle(Image *img,
			 unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2,
			 unsigned char R, unsigned char G, unsigned char B, unsigned char A) {
    if (!convertImage(img, Image::CONVENIENT)) return false ;

    unsigned int width = img->getWidth() ;
    unsigned int height = img->getHeight() ;
    if (x1>width-1) x1=width-1 ; if (x2>width-1) x2=width-1 ;
    if (x1<0) x1=0 ; if (x2<0) x2=0 ;
    if (y1>height-1) y1=height-1 ; if (y2>height-1) y2=height-1 ;
    if (y1<0) y1=0 ; if (y2<0) y2=0 ;

    unsigned int bytesPerPixel = img->getBytesPerPixel() ;
    unsigned char *data = (unsigned char *)img->getData() ;
    int lineWidth = width*bytesPerPixel ;
    Image::Encoding encoding = img->getEncoding() ;
    unsigned char argb[4] = {A, R, G, B} ;

    unsigned char *ptr = data + x1*bytesPerPixel + y1*lineWidth ;
    int offset = (y2-y1)*lineWidth ;
    for (unsigned int i=x1; i<=x2; ++i) {
	 drawPixel(ptr, argb, encoding) ;
	 drawPixel(ptr+offset, argb, encoding) ;
	 ptr += bytesPerPixel ;
    }

    ptr = data + x1*bytesPerPixel + y1*lineWidth ;
    offset = (x2-x1)*bytesPerPixel ;
    for (unsigned int i=y1; i<=y2; ++i) {
	 drawPixel(ptr, argb, encoding) ;
	 drawPixel(ptr+offset, argb, encoding) ;
	 ptr += lineWidth ;
    }

    return true ;
  }

  // ------------------------------------------------------------------

  bool
  paintImageRegion(Image *img,
			    unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2,
			    unsigned char R, unsigned char G, unsigned char B, unsigned char A) {
    if (!convertImage(img, Image::CONVENIENT)) return false ;

    unsigned int bytesPerPixel = img->getBytesPerPixel() ;
    unsigned int width = img->getWidth() ;
    unsigned int height = img->getHeight() ;

    if (x1>width-1) x1=width-1 ; if (x2>width-1) x2=width-1 ;
    if (x1<0) x1=0 ; if (x2<0) x2=0 ;

    if (y1>height-1) y1=height-1 ; if (y2>height-1) y2=height-1 ;
    if (y1<0) y1=0 ; if (y2<0) y2=0 ;

    unsigned char *data = (unsigned char *)img->getData() ;
    int lineWidth = width*bytesPerPixel ;

    unsigned char newvalues[4] = {A, R, G, B} ;

    Image::Encoding e = img->getEncoding() ;

    if (e==Image::A) {
	 for (unsigned int y=y1; y<=y2; ++y) {
	   unsigned char *ptr = data + lineWidth*y + x1 -1 ;
	   for (unsigned int x=x1; x<=x2; ++x) *ptr++ = A ;
	 }
    } else if (e==Image::L) {
	 int v = (int)(0.11*B + 0.59*G + 0.3*R) ;
	 for (unsigned int y=y1; y<=y2; ++y) {
	   unsigned char *ptr = data + lineWidth*y + x1 -1 ;
	   for (unsigned int x=x1; x<=x2; ++x)
		*ptr++ = (unsigned char)v ;
	 }
    }
    else if (e==Image::RGB565)
    {
	    uint16_t v = (uint16_t) (((R >> 3) & 0x1f) << 11)  |
		    (((G >> 2) & 0x3f) << 5)   |
		    (((B >> 3) & 0x1f) << 0);
	    for (unsigned int y=y1; y<=y2; ++y)
	    {
		    uint16_t *ptr =
			    (uint16_t *)(data + lineWidth*y + x1*bytesPerPixel);
		    for (unsigned int x=x1; x<=x2; ++x)
		    {
			    *ptr++ = v;
		    }
	    }
    }
    else {
	 unsigned int offset = 0 ;
	 if (e==Image::RGBA) {
	   newvalues[0] = R;
	   newvalues[1] = G;
	   newvalues[2] = B;
	   newvalues[3] = A;
	 } else if (e==Image::RGB) {
	   offset = 1 ;
	 }
	 for (unsigned int y=y1; y<=y2; ++y) {
	   unsigned char *ptr = data + lineWidth*y + x1*bytesPerPixel ;
	   for (unsigned int x=x1; x<=x2; ++x) {
		memmove(ptr, newvalues+offset, bytesPerPixel) ;
		ptr += bytesPerPixel ;
	   }
	 }
    }

    return true ;
  }

  // ------------------------------------------------------------------

  bool
  paintImage(Image *img,
		   unsigned char R, unsigned char G, unsigned char B, unsigned char A) {
    unsigned int width = img->getWidth() ;
    unsigned int height = img->getHeight() ;
    if (!width || !height) return false ;
#if defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_4) 
    if (img->getEncoding()==Image::ARGB) {
	 vImage_Buffer vimg = {img->getData(),height,width,width*4} ;
	 Pixel_8888 color = {A,R,G,B} ;
	 return vImageBufferFill_ARGB8888(&vimg, color, kvImageNoFlags)==kvImageNoError ;
    }
#endif
    return paintImageRegion(img, 0,0,width-1,height-1, R,G,B,A) ;
  }

  // ------------------------------------------------------------------

  static void
  place(unsigned char *src, const unsigned sWidth, const unsigned sHeight,
	   unsigned char *dst, const unsigned dWidth, const unsigned dHeight,
	   int xD, int yD,
	   const unsigned int bytesPerPixel) {
    if (!src || !sWidth || !sHeight || !dst || !dWidth || !dHeight) return ;

    if (xD==0 && yD==0 && sWidth==dWidth && sHeight==dHeight) {
	 memmove(dst, src, sWidth*sHeight*bytesPerPixel) ;
	 return ;
    }

    int xS=0, yS=0 ;
    int w = sWidth ;
    int h = sHeight ;

    if (xD<0) { xS-=xD ; w+=xD ; xD=0 ; }
    if (yD<0) { yS-=yD ; h+=yD ; yD=0 ; }

    if ((unsigned int)xD>=dWidth || (unsigned int)yD>=dHeight) return ;

    if ((unsigned int)(xD+w)>=dWidth) { w=dWidth-xD ; }
    if ((unsigned int)(yD+h)>=dHeight) { h=dHeight-yD ; }

#if 0
    std::cerr << "place: " << xS << "," << yS << " (" << sWidth << "x" << sHeight << ")" ;
    std::cerr << " -- " << w << "x" << h << " --> " ;
    std::cerr << xD << "," << yD << " (" << dWidth << "x" << dHeight << ")" << std::endl ;
#endif
  
    unsigned char *pSrc = (unsigned char *)src + (yS*sWidth + xS)*bytesPerPixel ;
    unsigned char *pDst = dst + (yD*dWidth + xD)*bytesPerPixel ;
    const unsigned int realSWidth = sWidth*bytesPerPixel ;
    const unsigned int realDWidth = dWidth*bytesPerPixel ;
    const unsigned int realWidth = w*bytesPerPixel ;

    while (h--) {
	 memmove(pDst, pSrc, realWidth) ;
	 pSrc+=realSWidth ;
	 pDst+=realDWidth ;
    }
  }

  // ------------------------------------------------------------------

  bool
  drawImageInImage(Image *src, Image *dst, int x, int y) {
    if (!dst->getData() || !dst->getSize()) return false ;

    // olivier: why?
    // nicolas: because it won't work with JPEG or PNG images, for example...
    if (!convertImage(dst, Image::CONVENIENT))
	 return false ;

    unsigned int bytesPerPixel = dst->getBytesPerPixel() ;

    Image tmp(*src) ;
    Image::Encoding encoding = dst->getEncoding() ;
    if (tmp.getEncoding() != encoding)
	 if (!convertImage(&tmp, encoding)) return false ;

    place((unsigned char *)tmp.getData(), tmp.getWidth(), tmp.getHeight(),
		(unsigned char *)dst->getData(), dst->getWidth(), dst->getHeight(),
		x, y,
		bytesPerPixel) ;

    return true ;
  }

  // ------------------------------------------------------------------

  bool
  blendImages(Image *src1, Image *src2, Image *dst, float blending) {
    unsigned int width=src1->getWidth(), height=src1->getHeight() ;
    if (width!=src2->getWidth() || height!=src2->getHeight()) return false ;

    Image tmp1(*src1), tmp2(*src2) ;

    Image::Encoding esrc1 = tmp1.getEncoding() ;
    Image::Encoding esrc2 = tmp2.getEncoding() ;
    Image::Encoding encoding = esrc1 ;
    if (esrc1!=esrc2 || !tmp1.encodingIsConvenient()) {
	 encoding = Image::ARGB ;
	 convertImage(&tmp1, encoding) ;
	 convertImage(&tmp2, encoding) ;
    }

    unsigned int bytesPerPixel = tmp1.getBytesPerPixel() ;
    unsigned int nbpixels = width*height ;
    unsigned int size = nbpixels*bytesPerPixel ;
    unsigned char *data = Image::AllocMem(size) ;
    float nb = 1.0-blending ;
    unsigned char *ptr1=tmp1.getData(), *ptr2=tmp2.getData(), *ptr=data ;
    for (int i=size; i; --i) {
	 unsigned char v = (unsigned char)(nb*(*(ptr1++))+blending*(*(ptr2++))) ;
	 *(ptr++) = v ;
    }
   
    dst->setEncoding(encoding) ;
    dst->setDims(width, height) ;
    dst->setData(data, size, Image::FREEMEM) ;

    return true ;
  }

  // ------------------------------------------------------------------

  bool
  blendImages(Image *src1, Image *src2, Image *dst) {
    unsigned int wsrc1=src1->getWidth(), hsrc1=src1->getHeight() ;
    unsigned int wsrc2=src2->getWidth(), hsrc2=src2->getHeight() ;
    if (wsrc1!=wsrc2 || hsrc1!=hsrc2) return false ;   

    Image::Encoding esrc2 = src2->getEncoding() ;
    if (esrc2!=Image::ARGB) return false ;

    Image tmp1(*src1) ;
    Image::Encoding esrc1 = tmp1.getEncoding() ;
    if (esrc1!=Image::ARGB && esrc1!=Image::RGB && esrc1!=Image::L) {
	 esrc1 = Image::ARGB ;
	 if (!convertImage(&tmp1, esrc1)) return false ;
    }

    unsigned int bytesPerPixel = tmp1.getBytesPerPixel() ;
    unsigned int nbpixels = wsrc1*hsrc1 ;
    unsigned int size = nbpixels*bytesPerPixel ;
    unsigned char *data = Image::AllocMem(size) ;
    unsigned char *ptr1=tmp1.getData(), *ptr2=src2->getData(), *ptr=data ;

    if (esrc1==Image::ARGB) {
	 for (unsigned int i=0; i<nbpixels; ++i) {
	   float alpha = ptr2[0]/255.0 ;
	   float omalpha = 1.0 - alpha ;
	   ptr[0] = ptr1[0] ;
	   for (unsigned int c=1; c<4; ++c)
		ptr[c] = (unsigned char)(omalpha*ptr1[c]+alpha*ptr2[c]) ;
	   ptr1 += 4 ;
	   ptr2 += 4 ;
	   ptr += 4 ;
	 }
    } else if (esrc1==Image::RGB) {
	 for (unsigned int i=0; i<nbpixels; ++i) {
	   float alpha = ptr2[0]/255.0 ;
	   float omalpha = 1.0 - alpha ;
	   for (unsigned int c=0; c<3; ++c)
		ptr[c] = (unsigned char)(omalpha*ptr1[c]+alpha*ptr2[c+1]) ;
	   ptr1 += 3 ;
	   ptr2 += 4 ;
	   ptr += 3 ;
	 }
    } else { // L
	 for (unsigned int i=0; i<nbpixels; ++i) {
	   float alpha = ptr2[0]/255.0 ;
	   float omalpha = 1.0 - alpha ;
	   float l = omalpha*(*ptr1) + alpha*(0.11*ptr2[3] + 0.59*ptr2[2] + 0.3*ptr2[1]) ;
	   *ptr = (unsigned char)l ;
	   ptr1 ++ ;
	   ptr2 += 4 ;
	   ptr ++ ;
	 }
    } 

    dst->setEncoding(esrc1) ;
    dst->setDims(wsrc1, hsrc1) ;
    dst->setData(data, size, Image::FREEMEM) ;  

    return true ;
  }

  // ------------------------------------------------------------------

}
