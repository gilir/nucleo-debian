/*
 *
 * nucleo/image/processing/chromakeying/ChromaKeyingFilter.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/processing/chromakeying/ChromaKeyingFilter.H>
#include <nucleo/image/encoding/Conversion.H>

#include <cmath>

namespace nucleo {

  // ---------------------------------------------------------------------------

  bool
  ChromaKeyingFilter::getKey(Image *img) {
    Image::Encoding e = img->getEncoding() ;
    if (e!=Image::ARGB && !convertImage(img,Image::ARGB)) return false ;

    unsigned int width = img->getWidth() ;
    unsigned int height = img->getHeight() ;
    unsigned int nbpixels = width*height ;
    unsigned char *image = img->getData() ;

    unsigned int Rm=0, Gm=0, Bm=0 ;
    unsigned char *ptr = image ;
    for (unsigned int i=0; i<nbpixels; ++i) {
	 Rm += *(ptr+1) ;
	 Gm += *(ptr+2) ;
	 Bm += *(ptr+3) ;
	 ptr += 4 ;
    }

    key[RED] = Rm/nbpixels ;
    key[GREEN] = Gm/nbpixels ;
    key[BLUE] = Bm/nbpixels ;

    float Vr=0.0, Vb=0.0, Vg=0.0 ;
    ptr = image ;

    for (unsigned int i=0; i<nbpixels; ++i) {
	 Vr+=pow((float)(key[RED]-*(ptr+1)), 2) ;
	 Vg+=pow((float)(key[GREEN]-*(ptr+2)), 2) ;
	 Vb+=pow((float)(key[BLUE]-*(ptr+3)), 2) ;
	 ptr += 4 ;
    }

    float ETr=sqrt(Vr/nbpixels);
    float ETg=sqrt(Vg/nbpixels);
    float ETb=sqrt(Vb/nbpixels);
    threshold = 3*((int)ETr+(int)ETg+(int)ETb)/3 + 2 ;

    return true ;
  }

  // ---------------------------------------------------------------------------

  bool
  ChromaKeyingFilter::filter(Image *img) {
    Image::Encoding e = img->getEncoding() ;
    if (e!=Image::ARGB && !convertImage(img,Image::ARGB)) return false ;

    const unsigned int width = img->getWidth() ;
    const unsigned int height = img->getHeight() ;
    const unsigned int nbpixels = width*height ;

    const float bt2=blackThreshold*1.1 ;

    unsigned char *ptr = (unsigned char *)img->getData() ;

    for (unsigned int i=0; i<nbpixels; ++i) {
	 const float R = ptr[1] ;
	 const float G = ptr[2] ;
	 const float B = ptr[3] ;
	 const float lum = R+G+B ;
	 if (lum<blackThreshold) {
	   *ptr=transparency ;
	 } else {
	   const float r=(R/lum)*255 ;
	   const float g=(G/lum)*255 ;
	   const float b=(B/lum)*255 ;
	   int dr=(int)(r-key[RED]) ; if (dr<0) dr=-dr ;
	   int dg=(int)(g-key[GREEN]) ; if (dg<0) dg=-dg ;
	   int db=(int)(b-key[BLUE]) ; if (db<0) db=-db ;
	   unsigned char value = transparency ;
	   if (lum>bt2) {
		if (dr<threshold && dg<threshold && db<threshold) value = 0 ;
	   } else if (lum>blackThreshold) {
		int toto = (dr+dg+db) ;
		int half = transparency/2 ;
		value = (unsigned char)(half - toto*threshold/half) ; 
	   }
	   *ptr = value ;
	 }

	 ptr+=4 ;   
    }	
  
    return true ;
  } 

  // ---------------------------------------------------------------------------

}
