#ifndef n_RGBAL_H
#define n_RGBAL_H

/*
 *
 * nucleo/image/encoding/RGBAL.H --
 *
 * Copyright (C) Nicolas Roussel, Olivier Chapuis
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <stdint.h>
#include <cstring>

#if defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_4) 
#include <Accelerate/Accelerate.h>
#endif

namespace nucleo {

  // -------------------------------------------------------------
  // L

  static inline void
  L2RGB(unsigned int w, unsigned int h,
	   unsigned char *src, unsigned char *dst) {
    for (unsigned int i=w*h; i; --i) {
	 memset(dst, *src, 3) ;
	 src++ ;
	 dst += 3 ;
    }
  }

  static inline void
  L2ARGB(unsigned int w, unsigned int h,
	    unsigned char *src, unsigned char *dst) {
    for (unsigned int i=w*h; i; --i) {
	 *dst++ = 255 ;
	 memset(dst, *src, 3) ;
	 src++ ;
	 dst += 3 ;
    }
  }

  static inline void
  L2RGBA(unsigned int w, unsigned int h,
	    unsigned char *src, unsigned char *dst) {
    for (unsigned int i=w*h; i; --i) {
      memset(dst, *src, 3) ;
      dst += 3 ;
      *dst++ = 255 ;
      src++ ;
    }
  }

  static inline void
  L2RGB565(unsigned int w, unsigned int h,
		 unsigned char *src, unsigned char *dst) {
    uint16_t *cdst = (uint16_t *) dst;
    for (unsigned int i=w*h; i; --i) {
	 *cdst = (((*src >> 3) & 0x1f) << 11)  |
	   (((*src >> 2) & 0x3f) << 5)   |
	   (((*src >> 3) & 0x1f) << 0);
	 cdst++;
	 src++;
    }
  }

  // -------------------------------------------------------------
  // RGB

  static inline void
  RGB2L(unsigned int w, unsigned int h,
	   unsigned char *src, unsigned char *dst) {
    for (unsigned int i=w*h; i; i--) {
	 double r=src[0], g=src[1], b=src[2] ;
	 *dst++ = (unsigned char)(0.11*b + 0.59*g + 0.3*r) ;
	 src += 3 ;
    }
  }

  static inline void
  RGB2ARGB(unsigned int w, unsigned int h,
		 unsigned char *src, unsigned char *dst) {
#if defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_4) 
    vImage_Buffer visrc = {src,h,w,w*3} ;
    vImage_Buffer vidst = {dst,h,w,w*4} ;
    vImage_Error error = vImageConvert_RGB888toARGB8888(&visrc, NULL,255, &vidst, 0, kvImageNoFlags) ;
    if (error == kvImageNoError) return ;
#endif

    for (unsigned int i=w*h; i; i--) {
	 *dst++ = 255 ;
	 memmove(dst, src, 3) ;
	 dst += 3 ;
	 src += 3 ;
    }
  }

  static inline void
  RGB2RGBA(unsigned int w, unsigned int h,
		 unsigned char *src, unsigned char *dst) {
    for (unsigned int i=w*h; i; i--) {
	 memmove(dst, src, 3) ;
	 dst += 3 ;
	 src += 3 ;
	 *dst++ = 255 ;
    }
  }

  static inline void RGB2RGB565(unsigned int w, unsigned int h,
						  unsigned char *src, unsigned char *dst) {
    uint16_t *cdst = (uint16_t *) dst;
    for (unsigned int i=w*h; i; --i) {
	 *cdst = (((src[0] >> 3) & 0x1f) << 11)  |
	   (((src[1] >> 2) & 0x3f) << 5)   |
	   (((src[2] >> 3) & 0x1f) << 0);
	 cdst++;
	 src += 3;
    }
  }

  // -------------------------------------------------------------
  // ARGB

  static inline void
  ARGB2L(unsigned int w, unsigned int h,
	    unsigned char *src, unsigned char *dst) {
    for (unsigned int i=w*h; i; i--) {
	 double r=src[1], g=src[2], b=src[3] ;
	 *dst++ = (unsigned char)(0.11*b + 0.59*g + 0.3*r) ;
	 src += 4 ;
    }
  }

  static inline void
  ARGB2RGB(unsigned int w, unsigned int h,
		 unsigned char *src, unsigned char *dst) {
#if defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_4) 
    vImage_Buffer visrc = {src,h,w,w*4} ;
    vImage_Buffer vidst = {dst,h,w,w*3} ;
    vImage_Error error = vImageConvert_ARGB8888toRGB888(&visrc, &vidst, kvImageNoFlags) ;
    if (error == kvImageNoError) return ;
#endif
    src++ ;
    for (unsigned int i=w*h; i; i--) {
	 memmove(dst, src, 3) ;
	 src += 4 ;
	 dst += 3 ;
    }
  }

  static inline void
  ARGB2RGBA(unsigned int w, unsigned int h,
		 unsigned char *src, unsigned char *dst) {
    for (unsigned int i=w*h; i; i--) {
      dst[3] = *src++;
      memmove(dst, src, 3) ;
      src += 3 ;
      dst += 4 ;
    }
  }

  static inline void
  ARGB2RGB565(unsigned int w, unsigned int h,
		    unsigned char *src, unsigned char *dst) {
    uint16_t *cdst = (uint16_t *) dst;
    for (unsigned int i=w*h; i; --i) {
	 *cdst = (((src[1] >> 3) & 0x1f) << 11)  |
	   (((src[2] >> 2) & 0x3f) << 5)   |
	   (((src[3] >> 3) & 0x1f) << 0);
	 cdst++;
	 src += 4;
    }
  }

  // -------------------------------------------------------------
  // RGBA

  static inline void
  RGBA2L(unsigned int w, unsigned int h,
	    unsigned char *src, unsigned char *dst) {
    for (unsigned int i=w*h; i; i--) {
	 double r=src[0], g=src[1], b=src[2] ;
	 *dst++ = (unsigned char)(0.11*b + 0.59*g + 0.3*r) ;
	 src += 4 ;
    }
  }

  static inline void
  RGBA2RGB(unsigned int w, unsigned int h,
		 unsigned char *src, unsigned char *dst) {
    for (unsigned int i=w*h; i; i--) {
	 memmove(dst, src, 3) ;
	 src += 4 ;
	 dst += 3 ;
    }
  }

  static inline void
  RGBA2ARGB(unsigned int w, unsigned int h,
		 unsigned char *src, unsigned char *dst) {
    for (unsigned int i=w*h; i; i--) {
      *dst++ = src[3] ;
      memmove(dst, src, 3) ;
      src += 4 ;
      dst += 3 ;
    }
  }

  static inline void
  RGBA2RGB565(unsigned int w, unsigned int h,
		    unsigned char *src, unsigned char *dst) {
    uint16_t *cdst = (uint16_t *) dst;
    for (unsigned int i=w*h; i; --i) {
	 *cdst = (((src[0] >> 3) & 0x1f) << 11)  |
	   (((src[1] >> 2) & 0x3f) << 5)   |
	   (((src[2] >> 3) & 0x1f) << 0);
	 cdst++;
	 src += 4;
    }
  }

  // -------------------------------------------------------------
  // RGB565

  static inline void 
  RGB5652L(unsigned int w, unsigned int h,
		 unsigned char *src, unsigned char *dst) {
    uint16_t *csrc = (uint16_t *) dst;
    for (unsigned int i=w*h; i; --i) {
	 double r,g,b;
	 r = (((*csrc >> 11) & 0x1f) << 3);
	 g = (((*csrc >> 5) & 0x3f) << 2);
	 b = ((*csrc & 0x1f) << 3);
	 *dst++ = (unsigned char)(0.11*b + 0.59*g + 0.3*r) ;
	 csrc++;
    }
  }

  static inline void
  RGB5652RGB(unsigned int w, unsigned int h,
		   unsigned char *src, unsigned char *dst) {
    uint16_t *csrc = (uint16_t *) dst;
    for (unsigned int i=w*h; i; --i) {
	 *dst++ = (((*csrc >> 11) & 0x1f) << 3);
	 *dst++ = (((*csrc >> 5) & 0x3f) << 2);
	 *dst++ = ((*csrc & 0x1f) << 3);
	 csrc++;
    }
  }

  static inline void
  RGB5652ARGB(unsigned int w, unsigned int h,
		    unsigned char *src, unsigned char *dst) {
    uint16_t *csrc = (uint16_t *) dst;
    for (unsigned int i=w*h; i; --i) {
	 *dst++ = 0xff;
	 *dst++ = ((*csrc >> 11) & 0x1f) << 3;
	 *dst++ = ((*csrc >> 5) & 0x3f) << 2;
	 *dst++ = (*csrc & 0x1f) << 3;
	 csrc++;
    }
  }

  static inline void
  RGB5652RGBA(unsigned int w, unsigned int h,
		    unsigned char *src, unsigned char *dst) {
    uint16_t *csrc = (uint16_t *) dst;
    for (unsigned int i=w*h; i; --i) {
	 *dst++ = ((*csrc >> 11) & 0x1f) << 3;
	 *dst++ = ((*csrc >> 5) & 0x3f) << 2;
	 *dst++ = (*csrc & 0x1f) << 3;
	 *dst++ = 0xff;
	 csrc++;
    }
  }

 //------------------------------------------------------

}

#endif
