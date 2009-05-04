/*
 *
 * nucleo/image/encoding/CoreGraphics.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/image/Image.H>
#include <nucleo/image/encoding/Conversion.H>

#include <ApplicationServices/ApplicationServices.h>

#include <stdexcept>
#include <iostream>

namespace nucleo {

  // -------------------------------------------------------------------------

  static inline int
  getIntVal(CFDictionaryRef dict, CFStringRef key, int val) {
    CFNumberRef n = (CFNumberRef)CFDictionaryGetValue(dict, key) ;
    if (n) CFNumberGetValue(n, kCFNumberIntType, &val) ;
    return val ;
  }

  static inline float
  getFloatVal(CFDictionaryRef dict, CFStringRef key, float val) {
    CFNumberRef n = (CFNumberRef)CFDictionaryGetValue(dict, key) ;
    if (n) CFNumberGetValue(n, kCFNumberFloatType, &val) ;
    return val ;
  }

  static bool
  cgPreamble(Image *img,
		   CGDataProviderRef *provider, CGImageSourceRef *source,
		   CFDictionaryRef *imgprops) {
    *provider = CGDataProviderCreateWithData(0, img->getData(), img->getSize(), 0) ;
    if (!*provider) return false ;

    CFMutableDictionaryRef srcprops = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
												    &kCFTypeDictionaryKeyCallBacks, 
												    &kCFTypeDictionaryValueCallBacks) ;
    if (!srcprops) {
	 CGDataProviderRelease(*provider) ;
	 return false ;
    }

    switch (img->getEncoding()) {
    case Image::JPEG: 
	 CFDictionarySetValue(srcprops, kCGImageSourceTypeIdentifierHint, kUTTypeJPEG) ;
	 break ;
    case Image::PNG:
	 CFDictionarySetValue(srcprops, kCGImageSourceTypeIdentifierHint, kUTTypePNG) ;
	 break ;
    default:
	 break ;
    }

    *source = CGImageSourceCreateWithDataProvider(*provider, srcprops) ;
    if (!*source) {
	 CFRelease(srcprops) ;
	 CGDataProviderRelease(*provider) ;
	 return false ;
    }

    *imgprops = CGImageSourceCopyPropertiesAtIndex(*source, 0, 0) ;
    if (!*imgprops) {
	 CFRelease(*source) ;
	 CFRelease(srcprops) ;
	 CGDataProviderRelease(*provider) ;
	 return false ;
    }

    CFRelease(srcprops) ;
    return true ;
  }

  static TimeStamp::inttype
  cgGetTimeStamp(CFDictionaryRef imgprops) {
    CFDictionaryRef exifd = (CFDictionaryRef)CFDictionaryGetValue(imgprops, kCGImagePropertyExifDictionary) ;
    if (!exifd) return TimeStamp::createAsInt() ;

    CFStringRef s = (CFStringRef)CFDictionaryGetValue(exifd, kCGImagePropertyExifDateTimeOriginal) ;
    if (!s) return TimeStamp::createAsInt() ;

    char timestamp[25] ;
    CFStringGetCString(s, timestamp, 20, kCFStringEncodingISOLatin1) ;
    // 2006:03:05 11:31:50
    // 012345678901234567890123
    // 2006-03-05T11:31:50.000Z
    timestamp[4] = timestamp[7] = '-' ;
    timestamp[10] = 'T' ;
    timestamp[13] = timestamp[16] = ':' ;
    timestamp[19] = '.' ;
    s = (CFStringRef)CFDictionaryGetValue(exifd, kCGImagePropertyExifSubsecTimeOrginal) ;
    if (s)
	 CFStringGetCString(s, timestamp+20, 4, kCFStringEncodingISOLatin1) ;
    else
	 strcpy(timestamp+20,"000") ;
    timestamp[23] = 'Z' ;
    timestamp[24] = '\0' ;
    return TimeStamp::createAsIntFromString(timestamp) ;
  }

  // -------------------------------------------------------------------------

  void
  cg_decode_meta(Image *img) {
    CGDataProviderRef provider = 0 ;
    CGImageSourceRef source = 0 ;
    CFDictionaryRef imgprops = 0 ;
    if (!cgPreamble(img, &provider, &source, &imgprops)) return ;
    int orientation = getIntVal(imgprops, kCGImagePropertyOrientation, 1) ;
    if (orientation<1 || orientation>8) orientation = 1 ;
    int width = getIntVal(imgprops, kCGImagePropertyPixelWidth, 0) ;
    int height = getIntVal(imgprops, kCGImagePropertyPixelHeight, 0) ;
    if (orientation>4) img->setDims(height,width) ; else img->setDims(width,height) ;
    img->setTimeStamp(cgGetTimeStamp(imgprops)) ;
    CFRelease(imgprops) ;
    CFRelease(source) ;
    CGDataProviderRelease(provider) ;
  }

  bool
  cg_decode(Image *src, Image *dst, Image::Encoding e, unsigned int quality) {
    CGDataProviderRef provider = 0 ;
    CGImageSourceRef source = 0 ;
    CFDictionaryRef imgprops = 0 ;
    if (!cgPreamble(src, &provider, &source, &imgprops)) {
	 return false ;
    }
    // CFShow(imgprops) ;

    int orientation = getIntVal(imgprops, kCGImagePropertyOrientation, 1) ;
    if (orientation<1 || orientation>8) orientation = 1 ;
    float xdpi = getFloatVal(imgprops, kCGImagePropertyDPIWidth, 72) ;
    float ydpi = getFloatVal(imgprops, kCGImagePropertyDPIHeight, 72) ;
    int width = getIntVal(imgprops, kCGImagePropertyPixelWidth, 0) ;
    int height = getIntVal(imgprops, kCGImagePropertyPixelHeight, 0) ;

    float x = (ydpi>xdpi) ? ydpi/xdpi : 1 ;
    float y = (xdpi>ydpi) ? xdpi/ydpi : 1 ;
    float w = x*width, h = y*height ;
    CGAffineTransform transforms[8] = {
        { x, 0, 0, y, 0, 0},  //  1 =  row 0 top, col 0 lhs  =  normal
        {-x, 0, 0, y, w, 0},  //  2 =  row 0 top, col 0 rhs  =  flip horizontal
        {-x, 0, 0,-y, w, h},  //  3 =  row 0 bot, col 0 rhs  =  rotate 180
        { x, 0, 0,-y, 0, h},  //  4 =  row 0 bot, col 0 lhs  =  flip vertical
        { 0,-x,-y, 0, h, w},  //  5 =  row 0 lhs, col 0 top  =  rot -90, flip vert
        { 0,-x, y, 0, 0, w},  //  6 =  row 0 rhs, col 0 top  =  rot 90
        { 0, x, y, 0, 0, 0},  //  7 =  row 0 rhs, col 0 bot  =  rot 90, flip vert
        { 0, x,-y, 0, h, 0}   //  8 =  row 0 lhs, col 0 bot  =  rotate -90
    };

    if (orientation>4)
	 dst->prepareFor(height,width,Image::ARGB) ;
    else
	 dst->prepareFor(width,height,Image::ARGB) ;
    dst->setTimeStamp(cgGetTimeStamp(imgprops)) ;

    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB() ;
    CGContextRef ctx = CGBitmapContextCreate(dst->getData(), dst->getWidth(), dst->getHeight(),
									8, dst->getWidth()*4, colorspace, 
									kCGImageAlphaPremultipliedFirst) ;
    CGRect rect = {{0, 0}, {width, height}} ;
    CGImageRef img = CGImageSourceCreateImageAtIndex(source, 0, 0) ;

#if DEBUG_CG_DECODE
    std::cerr << "status: " << CGImageSourceGetStatus(source) << std::endl ;
    std::cerr << "status(0): " << CGImageSourceGetStatusAtIndex(source, 0) << std::endl ;
#endif

    bool result = false ;
    if (img) {
	 if (orientation!=1) CGContextConcatCTM(ctx, transforms[orientation-1]) ;
#if DEBUG_CG_DECODE
	 std::cerr << "cg_decode, line " << __LINE__ << std::endl ;
	 std::cerr << "status: " << CGImageSourceGetStatus(source) << std::endl ;
	 std::cerr << "status(0): " << CGImageSourceGetStatusAtIndex(source, 0) << std::endl ;
#endif
	 CGContextDrawImage(ctx, rect, img) ;
#if DEBUG_CG_DECODE
	 std::cerr << "status: " << CGImageSourceGetStatus(source) << std::endl ;
	 std::cerr << "status(0): " << CGImageSourceGetStatusAtIndex(source, 0) << std::endl ;
	 // std::cerr << "cg_decode, line " << __LINE__ << std::endl ;
#endif
	 convertImage(dst, e, quality) ;
	 result = true ;
    }

    CGContextRelease(ctx) ;
    CGColorSpaceRelease(colorspace) ;
    CFRelease(imgprops) ;
    CGImageRelease(img) ;
    CFRelease(source) ;
    CGDataProviderRelease(provider) ;

    // std::cerr << "cg_decode, line " << __LINE__ << std::endl ;
    return result ;
  }

  bool
  cg_encode(Image *src, Image *dst, Image::Encoding e, unsigned int quality) {
    convertImage(src, Image::ARGB) ;
    CFDataRef indata = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,
										 src->getData(), src->getSize(), kCFAllocatorNull) ;

    CGDataProviderRef provider = CGDataProviderCreateWithCFData(indata) ;
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB() ;
    CGImageRef img = CGImageCreate(src->getWidth(), src->getHeight(), 
							8, 32, src->getWidth()*4, 
							colorspace, kCGImageAlphaPremultipliedFirst, 
							provider, NULL, false, 
							kCGRenderingIntentDefault) ;
    CGDataProviderRelease(provider) ;
    CGColorSpaceRelease(colorspace) ;

    CFMutableDataRef outdata = CFDataCreateMutable(kCFAllocatorDefault, 0) ;
    CFStringRef type = e==Image::JPEG ? kUTTypeJPEG : kUTTypePNG ;
    CGImageDestinationRef dest = CGImageDestinationCreateWithData(outdata, type, 1, 0) ;

    CFMutableDictionaryRef props = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
												 &kCFTypeDictionaryKeyCallBacks,
												 &kCFTypeDictionaryValueCallBacks);
    double q = (double)quality/100.0 ;
    CFNumberRef v = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &q);
    CFDictionarySetValue(props, kCGImageDestinationLossyCompressionQuality, v);
    CFRelease(v) ;

    std::string timestamp = TimeStamp::createAsStringFromInt(src->getTimeStamp()) ;
    // 2006-03-05T11:31:50.000Z
    // 012345678901234567890123
    // 2006:03:05 11:31:50
    timestamp[4] = timestamp[7] = ':' ;
    timestamp[10] = ' ' ;

    CFStringRef cfts = CFStringCreateWithBytes(kCFAllocatorDefault,
									  (const UInt8*)timestamp.c_str(), 19,
									  kCFStringEncodingISOLatin1, false) ;
    CFStringRef cfsts = CFStringCreateWithBytes(kCFAllocatorDefault,
									  (const UInt8*)timestamp.c_str()+20, 3,
									  kCFStringEncodingISOLatin1, false) ;
    CFMutableDictionaryRef exifd = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
												 &kCFTypeDictionaryKeyCallBacks,
												 &kCFTypeDictionaryValueCallBacks) ;
    CFDictionarySetValue(props, kCGImagePropertyExifDictionary, exifd);
    CFDictionarySetValue(exifd, kCGImagePropertyExifDateTimeOriginal, cfts) ;
    CFDictionarySetValue(exifd, kCGImagePropertyExifSubsecTimeOrginal, cfsts) ;
    CFRelease(cfts) ;
    CFRelease(exifd) ;

    CGImageDestinationAddImage(dest, img, props) ;
    CGImageDestinationFinalize(dest) ;

    CFIndex length = CFDataGetLength(outdata) ;
    dst->setEncoding(e) ;
    dst->setData(Image::AllocMem(length), length, Image::FREEMEM) ;
    dst->setTimeStamp(src->getTimeStamp()) ;
    dst->setDims(src->getWidth(), src->getHeight()) ;
    CFDataGetBytes(outdata, CFRangeMake(0, length), dst->getData()) ;

    CFRelease(indata) ;
    CFRelease(outdata) ;
    CFRelease(dest) ;
    CFRelease(props) ;
    CGImageRelease(img) ;

    return true ;
  }

  // -------------------------------------------------------------------------

}
