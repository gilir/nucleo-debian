/*
 *
 * nucleo/image/encoding/YpCbCr420.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/Image.H>

namespace nucleo {

  void
  xRGB2YpCbCr420(Image *img) {
    unsigned int src_w=img->getWidth(), src_h=img->getHeight() ;
    unsigned int dst_w=src_w&0xFFF0, dst_h=src_h&0xFFF0 ;
    unsigned int dst_nbpixels = dst_w*dst_h ;
    unsigned int dst_size = (int)(dst_nbpixels*1.5) ;

    unsigned char *src = (unsigned char *)img->getData() ;
    unsigned char *dst = (unsigned char *)Image::AllocMem(dst_size) ;

    unsigned char *cbplane = (unsigned char *)Image::AllocMem(dst_size) ;
    unsigned char *crplane = (unsigned char *)Image::AllocMem(dst_size) ;

    unsigned char *ypplane = dst ;
    unsigned char *cbptr = cbplane ;
    unsigned char *crptr = crplane ;

    unsigned int bpp = img->getBytesPerPixel() ;

    for (unsigned int y=0; y<dst_h; ++y) {
	 unsigned char *ptr = src + (y*src_w*bpp) ;
	 for (unsigned int x=0; x<dst_w; ++x) {
	   if (bpp==4) ptr++ ;
	   unsigned char Rp=*ptr++ ;
	   unsigned char Gp=*ptr++ ;
	   unsigned char Bp=*ptr++ ;	   
	   *ypplane++ = (unsigned char)(16 + (65.738*Rp + 129.057*Gp +25.064*Bp)/256.0) ;
	   *cbptr++ = (unsigned char)(128 + (-37.945*Rp - 74.494*Gp + 112.439*Bp)/256.0) ;
	   *crptr++ = (unsigned char)(128 + (112.439*Rp - 94.154*Gp - 18.285*Bp)/256.0) ;
	 }
    }

    cbptr = dst + dst_nbpixels ;
    crptr = dst + (int)(dst_nbpixels*1.25) ;

    for (unsigned int y=0; y<dst_h;) {
	 for (unsigned int x=0; x<dst_w;) {
	   int cb1 = cbplane[y*dst_w+x] ;
	   int cb2 = cbplane[y*dst_w+x+1] ;
	   int cb3 = cbplane[(y+1)*dst_w+x] ;
	   int cb4 = cbplane[(y+1)*dst_w+x+1] ;
	   *cbptr++ = (unsigned char)((cb1+cb2+cb3+cb4)/4) ;

	   int cr1 = crplane[y*dst_w+x] ;
	   int cr2 = crplane[y*dst_w+x+1] ;
	   int cr3 = crplane[(y+1)*dst_w+x] ;
	   int cr4 = crplane[(y+1)*dst_w+x+1] ;
	   *crptr++ = (unsigned char)((cr1+cr2+cr3+cr4)/4) ;

	   x += 2 ;
	 }
	 y += 2 ;
    }

    Image::FreeMem(&cbplane) ;
    Image::FreeMem(&crplane) ;

    img->setEncoding(Image::YpCbCr420) ;
    img->setData(dst, dst_size, Image::FREEMEM) ;
    img->setDims(dst_w, dst_h) ;
  }

  // ------------------------------------------------------------------

  /*
   * Turn a YUV4:2:0 block into an RGB block (this code comes from
   * ov511.c) 
   *
   * Color space conversion coefficients taken from the excellent
   * http://www.inforamp.net/~poynton/ColorFAQ.html
   * In his terminology, this is a CCIR 601.1 YCbCr -> RGB.
   * Y values are given for all 4 pixels, but the U (Pb)
   * and V (Pr) are assumed constant over the 2x2 block.
   *
   * To avoid floating point arithmetic, the color conversion
   * coefficients are scaled into 16.16 fixed-point integers.
   * They were determined as follows:
   *
   *	double brightness = 1.0;  (0->black; 1->full scale) 
   *	double saturation = 1.0;  (0->greyscale; 1->full color)
   *	double fixScale = brightness * 256 * 256;
   *	int rvScale = (int)(1.402 * saturation * fixScale);
   *	int guScale = (int)(-0.344136 * saturation * fixScale);
   *	int gvScale = (int)(-0.714136 * saturation * fixScale);
   *	int buScale = (int)(1.772 * saturation * fixScale);
   * int yScale = (int)(fixScale);
   */
  
  /* LIMIT: convert a 16.16 fixed-point value to a byte, with clipping. */
#define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))

  void
  YpCbCr4202xRGB(Image *img, Image::Encoding encoding) {
    unsigned int bpp = img->getBytesPerPixel(encoding) ;

    const unsigned int w=img->getWidth(), h=img->getHeight() ;
    const int numpix = w*h ;

    unsigned char *pIn0 = img->getData() ;
    unsigned int size = numpix*bpp ;
    unsigned char *buffer = (unsigned char *)Image::AllocMem(size) ;

    unsigned char *pY = pIn0 ;
    unsigned char *pU = pY + numpix ;
    unsigned char *pV = pU + numpix / 4 ;
    unsigned char *pOut = buffer ;

    const int o = bpp-3 ;
    const int skip = bpp * w + o ;

    const int rvScale = 91881 ;
    const int guScale = -22553 ;
    const int gvScale = -46801 ;
    const int buScale = 116129 ;
    const int yScale  = 65536 ;

    for (unsigned int j = 0; j <= h - 2; j += 2) {
	 for (unsigned int i = 0; i <= w - 2; i += 2) {
	   int yTL=(*pY)*yScale, yTR=(*(pY + 1))*yScale ;
	   int yBL=(*(pY + w))*yScale, yBR=(*(pY + w + 1))*yScale ;

	   int u = (*pU++) - 128 ;
	   int v = (*pV++) - 128 ;
	   int g = guScale * u + gvScale * v;
	   int r = buScale * u;
	   int b = rvScale * v;

	   // Write out top two pixels
	   pOut[o] = LIMIT(b+yTL); pOut[o+1] = LIMIT(g+yTL); pOut[o+2] = LIMIT(r+yTL) ;
	   pOut[o+bpp] = LIMIT(b+yTR); pOut[o+bpp+1] = LIMIT(g+yTR); pOut[o+bpp+2] = LIMIT(r+yTR) ;

	   // Skip down to next line to write out bottom two pixels
	   pOut[skip] = LIMIT(b+yBL); pOut[skip+1] = LIMIT(g+yBL); pOut[skip+2] = LIMIT(r+yBL) ;
	   pOut[skip+bpp] = LIMIT(b+yBR); pOut[skip+bpp+1] = LIMIT(g+yBR); pOut[skip+bpp+2] = LIMIT(r+yBR) ;

	   pY += 2;
	   pOut += 2 * bpp ;
	 }
	 pY += w ;
	 pOut += w * bpp ;
    }

    img->setEncoding(Image::RGB) ;
    img->setData(buffer, size, Image::FREEMEM) ;
  }

}
