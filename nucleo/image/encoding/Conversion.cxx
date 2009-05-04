/*
 *
 * nucleo/image/encoding/Conversion.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/encoding/Conversion.H>

#include <nucleo/image/encoding/RGBAL.H>
#include <nucleo/image/encoding/YpCbCr420.H>
#include <nucleo/image/encoding/PAM.H>

#if defined(__APPLE__)
#include <nucleo/image/encoding/CoreGraphics.H>
#endif

#if HAVE_LIBJPEG
#include <nucleo/image/encoding/JPEG.H>
#endif
#if HAVE_LIBPNG
#include <nucleo/image/encoding/PNGenc.H>
#endif

#include <cstring>

#define DEBUG_LEVEL 0

namespace nucleo {

  // ------------------------------------------------------------------

  bool
  convertImage(Image *src, Image *dst, Image::Encoding dst_encoding, unsigned int quality) {
    Image::Encoding src_encoding = src->getEncoding() ;

    bool nothingTodo =
	 src_encoding==dst_encoding
	 || dst_encoding==Image::PREFERRED
	 || (dst_encoding==Image::CONVENIENT && Image::encodingIsConvenient(src_encoding)) ;

    if (nothingTodo) {
	 *dst = *src ;
	 // make sure that width and height are known (useful for JPEG,
	 // PNG and PAM data that need to be parsed)
	 dst->getWidth() ;
	 return true ;
    }

    // std::cerr << "convertImage: [" << src->getDescription() << "] -- " << Image::getEncodingName(dst_encoding) << " --> [" << dst->getDescription() << "]" << std::endl ;

    unsigned int width = src->getWidth() ;
    unsigned int height = src->getHeight() ;
    
    bool ok = true ;

    switch (src_encoding) {

    case Image::L:
	 switch (dst_encoding) {
	 case Image::RGB:
	   dst->prepareFor(width, height, dst_encoding) ;
	   L2RGB(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::ARGB:
	   dst->prepareFor(width, height, dst_encoding) ;
	   L2ARGB(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::RGBA:
	   dst->prepareFor(width, height, dst_encoding) ;
	   L2RGBA(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::RGB565:
	   dst->prepareFor(width, height, dst_encoding) ;
	   L2RGB565(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::JPEG:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBJPEG
	   ok = jpeg_encode(src, dst, quality) ;
#else
	   ok = false ;
#endif
	   break ;
	 case Image::PNG:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBPNG
	   ok = png_encode(src, dst, quality) ;
#else
	   ok = false ;
#endif
	   break ;
	 case Image::PAM:
	   ok = pam_encode(src, dst) ;
	   break ;
	 default: ok = false ;
	 }
	 break ;

    case Image::RGB:
	 switch (dst_encoding) {
	 case Image::L:
	   dst->prepareFor(width, height, dst_encoding) ;
	   RGB2L(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::ARGB:
	   dst->prepareFor(width, height, dst_encoding) ;
	   RGB2ARGB(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::RGBA:
	   dst->prepareFor(width, height, dst_encoding) ;
	   RGB2RGBA(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::RGB565:
	   dst->prepareFor(width, height, dst_encoding) ;
	   RGB2RGB565(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::YpCbCr420:
	   dst->linkDataFrom(*src) ;
	   xRGB2YpCbCr420(dst) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::JPEG:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBJPEG
	   ok = jpeg_encode(src, dst, quality) ;
#else
	   ok = false ;
#endif
	   break ;
	 case Image::PNG:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, Image::PNG, quality) ;
#elif HAVE_LIBPNG
	   ok = png_encode(src, dst, quality) ;
#else
	   ok = false ;
#endif
	   break ;
	 case Image::PAM:
	   ok = pam_encode(src, dst) ;
	   break ;
	 default: ok = false ;
	 }
	 break ;

    case Image::ARGB:
	 switch (dst_encoding) {
	 case Image::L:
	   dst->prepareFor(width, height, dst_encoding) ;
	   ARGB2L(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::RGB:
	   dst->prepareFor(width, height, dst_encoding) ;
	   ARGB2RGB(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::RGBA:
	   dst->prepareFor(width, height, dst_encoding) ;
	   ARGB2RGBA(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::RGB565:
	   dst->prepareFor(width, height, dst_encoding) ;
	   ARGB2RGB565(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::JPEG:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBJPEG
	   ok = jpeg_encode(src, dst, quality) ;
#else
	   ok = false ;
#endif
	   break ;
	 case Image::PNG:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBPNG
	   ok = png_encode(src, dst, quality) ;
#else
	   ok = false ;
#endif
	   break ;
	 case Image::PAM:
	   ok = pam_encode(src, dst) ;
	   break ;
	 case Image::YpCbCr420:
	   dst->linkDataFrom(*src) ;
	   xRGB2YpCbCr420(dst) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 default: ok = false ;
	 }
	 break ;
 
    case Image::RGBA:
	 switch (dst_encoding) {
	 case Image::L:
	   dst->prepareFor(width, height, dst_encoding) ;
	   RGBA2L(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::RGB:
	   dst->prepareFor(width, height, dst_encoding) ;
	   RGBA2RGB(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::ARGB:
	   dst->prepareFor(width, height, dst_encoding) ;
	   RGBA2ARGB(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::RGB565:
	   dst->prepareFor(width, height, dst_encoding) ;
	   RGBA2RGB565(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::JPEG:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBJPEG
	   ok = jpeg_encode(src, dst, quality) ;
#else
	   ok = false ;
#endif
	   break ;
	 case Image::PNG:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBPNG
	   ok = png_encode(src, dst, quality) ;
#else
	   ok = false ;
#endif
	   break ;
	 case Image::PAM:
	   ok = pam_encode(src, dst) ;
	   break ;
	 default: ok = false ;
	 }
	 break ;
 
    case Image::RGB565:
	 switch (dst_encoding) {
	 case Image::L:
	   dst->prepareFor(width, height, dst_encoding) ;
	   RGB5652L(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   ok = false ;
	   break ;
	 case Image::RGB:
	   dst->prepareFor(width, height, dst_encoding) ;
	   RGB5652RGB(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::ARGB:
	   dst->prepareFor(width, height, dst_encoding) ;
	   RGB5652ARGB(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::RGBA:
	   dst->prepareFor(width, height, dst_encoding) ;
	   RGB5652RGBA(width, height, src->getData(), dst->getData()) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break;
	 case Image::JPEG:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBJPEG
	   ok = jpeg_encode(src, dst, quality) ;
#else
	   ok = false ;
#endif
	   break ;
	 case Image::PNG:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBPNG
	   ok = png_encode(src, dst, quality) ;
#else
	   ok = false ;
#endif
	   break ;
	 case Image::PAM:
	   ok = pam_encode(src, dst) ;
	   break ;
	 default: ok = false ;
	 }
	 break ;

    case Image::YpCbCr422:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBJPEG
	 if (dst_encoding==Image::JPEG)
	   ok = jpeg_encode(src, dst, quality) ;
	 else
#else
	   ok = false ;
#endif
	 break ;

    case Image::YpCbCr420:
	 switch (dst_encoding) {
	 case Image::CONVENIENT:
	 case Image::RGB:
	   dst->linkDataFrom(*src) ;
	   YpCbCr4202xRGB(dst, Image::RGB) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::ARGB:
	   dst->linkDataFrom(*src) ;
	   YpCbCr4202xRGB(dst, dst_encoding) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   break ;
	 case Image::RGBA:
	 case Image::RGB565:
	 case Image::L: {
	   Image tmp ;
	   tmp.linkDataFrom(*src) ;
	   YpCbCr4202xRGB(&tmp, Image::RGB) ;
	   dst->prepareFor(width, height, dst_encoding) ;
	   dst->setTimeStamp(src->getTimeStamp()) ;
	   switch (dst_encoding)
	   {
	   case Image::L:
	     RGB2L(tmp.getWidth(), tmp.getHeight(), tmp.getData(), dst->getData()) ;
	     break;
	   case Image::RGBA:
	     RGB2RGBA(tmp.getWidth(), tmp.getHeight(), tmp.getData(), dst->getData()) ;
	     break;
	   case Image::RGB565:
	     RGB2RGB565(tmp.getWidth(), tmp.getHeight(), tmp.getData(), dst->getData()) ;
	     break;
	   default: ok = false ; // cannot happen
	   }
	 } break ;
	 case Image::JPEG:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBJPEG
	   ok = jpeg_encode(src, dst, quality) ;
#else
	   ok = false ;
#endif
	   break ;
	 case Image::PNG:
#if defined(__APPLE__)
	   ok = cg_encode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBPNG
	   ok = png_encode(src, dst, quality) ;
#else
	   ok = false ;
#endif
	   break ;
	 case Image::PAM:
	   ok = pam_encode(src, dst) ;
	   break ;
	 default: ok = false ;
	 }
	 break ;

    case Image::JPEG:
#if defined(__APPLE__)
	 ok = cg_decode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBJPEG
	 ok = jpeg_decode(src, dst, dst_encoding, quality) ;
#else
	 ok = false ;
#endif
	 break ;

    case Image::PNG:
#if defined(__APPLE__)
	 ok = cg_decode(src, dst, dst_encoding, quality) ;
#elif HAVE_LIBPNG
	 ok = png_decode(src, dst, dst_encoding, quality) ;
#else
	 ok = false ;
#endif
	 break ;

    case Image::PAM:
	 ok = pam_decode(src, dst, dst_encoding, quality) ;
	 break ;

    default: ok = false ;
    }

    if (ok) return true ;

    // std::cerr << "nucleo::convertImage failed (" << Image::getEncodingName(src_encoding) << " -> " << Image::getEncodingName(dst_encoding) << ")" << std::endl ;

    return false ;
  }

  // ------------------------------------------------------------------

  bool
  convertImage(Image *img, Image::Encoding dst_encoding, unsigned int quality) {
    Image dst ;
    bool res = convertImage(img, &dst, dst_encoding, quality) ;
    if (res) img->stealDataFrom(dst) ;
    return res ;
  }

}
