/*
 *
 * nucleo/image/Image.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/utils/FileUtils.H>

#include <nucleo/image/Image.H>

#include <nucleo/image/encoding/Conversion.H>
#if defined(__APPLE__)
#include <nucleo/image/encoding/CoreGraphics.H>
#else
#if HAVE_LIBJPEG
#include <nucleo/image/encoding/JPEG.H>
#endif
#if HAVE_LIBPNG
#include <nucleo/image/encoding/PNGenc.H>
#endif
#endif
#include <nucleo/image/encoding/PAM.H>

#include <cmath>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <sstream>

#define DEBUG_LEVEL 0

namespace nucleo {

  Image::StandardSize Image::StandardSizes[] = {	 
    {"16CIF",1408,1152}, // H.263
    {"PAL",768,576},
    {"4CIF",704,576},    // H.263
    {"VGA",640,480},
    {"PAL2",384,288},    // Non-standard name...
    {"CIF",352,288},     // H.263
    {"SIF",320,240},
    {"U1",288,216},      // Non-standard name...
    {"PAL3",256,192},    // Non-standard name...
    {"U2",224,168},      // Non-standard name...
    {"PAL4",192,144},    // Non-standard name...
    {"QCIF",176,144},    // H.263
    {"QSIF",160,120},
    {"SQCIF",128,96},    // H.263
    {0,0,0}
  } ;

  struct ImageEncodingInfo {
    Image::Encoding encoding ;
    char *name ;
    unsigned int bpp ;
    char *mimetype ;
  } ;

  static ImageEncodingInfo encodings[] = {
    {Image::OPAQUE,     "OPAQUE",     0, "image/vnd.nucleo.OPAQUE"},
    {Image::PREFERRED,  "PREFERRED",  0, "image/vnd.nucleo.PREFERRED"},
    {Image::CONVENIENT, "CONVENIENT", 0, "image/vnd.nucleo.CONVENIENT"},

    {Image::L,          "L",          1, "image/vnd.nucleo.L"},
    {Image::A,          "A",          1, "image/vnd.nucleo.A"},
    {Image::RGB,        "RGB",        3, "image/vnd.nucleo.RGB"},
    {Image::ARGB,       "ARGB",       4, "image/vnd.nucleo.ARGB"},
    {Image::RGBA,       "RGBA",       4, "image/vnd.nucleo.RGBA"},
    {Image::RGB565,     "RGB565",     2, "image/vnd.nucleo.RGB565"},

    {Image::YpCbCr420,  "YpCbCr420",  0, "image/vnd.nucleo.YpCbCr420"},
    {Image::YpCbCr422,  "YpCbCr422",  2, "image/vnd.nucleo.YpCbCr422"},
    {Image::JPEG,       "JPEG",       0, "image/jpeg"},
    {Image::PNG,        "PNG",        0, "image/png"},
    {Image::PAM,        "PAM",        0, "image/vnd.nucleo.PAM"}
  } ;

  static inline void
  update_metadata(Image *img, Image::Encoding e) {
    switch (e) {
    case Image::PAM: pam_calcdims(img) ; break ;
#if defined(__APPLE__)
    case Image::JPEG:
    case Image::PNG: cg_decode_meta(img) ; break ;
#else
#if HAVE_LIBJPEG
    case Image::JPEG: jpeg_calcdims(img) ; break ;
#endif
#if HAVE_LIBPNG
    case Image::PNG: png_calcdims(img) ; break ;
#endif
#endif
    default: break ;
    }
  }

  std::string
  Image::getEncodingName(Image::Encoding e) {
    for (unsigned int i=0; i<sizeof(encodings)/sizeof(ImageEncodingInfo); ++i)
	 if (e==encodings[i].encoding) return encodings[i].name ;
    return "<unknown>" ;
  }

  std::string
  Image::getEncodingMimeType(Image::Encoding e) {
    for (unsigned int i=0; i<sizeof(encodings)/sizeof(ImageEncodingInfo); ++i)
	 if (e==encodings[i].encoding) return encodings[i].mimetype ;
    return "image/vnd.nucleo.OPAQUE" ;
  }

  Image::Encoding
  Image::getEncodingByName(const char *name) {
    for (unsigned int i=0; i<sizeof(encodings)/sizeof(ImageEncodingInfo); ++i)
	 if (!strcmp(name,encodings[i].name)) return encodings[i].encoding ;
    return OPAQUE ;
  }

  Image::Encoding
  Image::getEncodingByMimeType(const char *mimetype) {
    for (unsigned int i=0; i<sizeof(encodings)/sizeof(ImageEncodingInfo); ++i)
	 if (!strcmp(mimetype,encodings[i].mimetype)) return encodings[i].encoding ;
    return OPAQUE ;
  }

  unsigned int
  Image::getBytesPerPixel(Image::Encoding e) {
    for (unsigned int i=0; i<sizeof(encodings)/sizeof(ImageEncodingInfo); ++i)
	 if (e==encodings[i].encoding) return encodings[i].bpp ;
    return 0 ;
  }

  bool
  Image::encodingIsConvenient(Encoding e) {
    return (e==Image::L) 
	 || (e==Image::A) 
	 || (e==Image::RGB) 
	 || (e==Image::ARGB)
	 || (e==Image::RGBA)
	 || (e==Image::RGB565) ;
  }

  // ---------------------------------------------------------------------------

  inline static char *getFreeMethodName(Image::FreeMethod m) {
    switch (m) {
    case Image::NONE: return "NONE" ;
    case Image::DELETE: return "DELETE" ;
    case Image::FREE: return "FREE" ;
    case Image::FREEMEM: return "FREEMEM" ;
    }
    return "?" ;
  }

  // ---------------------------------------------------------------------------

  Image::Image(Image& src) : _data(0), _fmethod(NONE) {
#if DEBUG_LEVEL>=1
    std::cerr << "Image::Image copy constructor" << std::endl ;
#endif
    _encoding = src._encoding ;
    setData(src._data, src._size, NONE) ;
    _timestamp = src._timestamp ;
    _width = src._width ;
    _height = src._height ;
  }

  Image&
  Image::operator = (Image& src) {
    if (&src!=this) {
#if DEBUG_LEVEL>=1
	 std::cerr << "Image::= copy assignement" << std::endl ;
#endif
	 _encoding = src._encoding ;
	 setData(src._data, src._size, NONE) ;
	 _timestamp = src._timestamp ;
	 _width = src._width ;
	 _height = src._height ;
    }
#if DEBUG_LEVEL>=1
    else std::cerr << "Image::= self copy assignement !" << std::endl ;
#endif
    return *this ;
  }

  // ---------------------------------------------------------------------------

  void
  Image::prepareFor(unsigned int w, unsigned int h, Encoding e) {
    _width = w ; _height = h ;
    _encoding = e ;

    unsigned int size = (e==YpCbCr420) ? (unsigned int)(ceil(w*h*1.5)) : w*h*getBytesPerPixel(e) ;
    if (!size || _size==size) return ;

    // std::cerr << "prepareFor: allocating " << size << " bytes" << std::endl ;
    unsigned char *ptr = AllocMem(size) ;
    setData(ptr, size, FREEMEM) ;
  }

  // ---------------------------------------------------------------------------

  void
  Image::setData(unsigned char *d, unsigned int s, FreeMethod m) {
#if DEBUG_LEVEL>=2
    std::cerr << "Image::setData (" << std::hex << (int)_data << std::dec << " " << _size << " " << getFreeMethodName(_fmethod) << ")" ;
    std::cerr << " --> " ;
    std::cerr << "(" << std::hex << (int)d << std::dec << " " << s << " " << getFreeMethodName(m) << ")" << std::endl ;
#endif

    if (_data==d) {
	 if (!_data) {
	   _fmethod = NONE ;
	  _size = 0 ;
	 } else if (_fmethod!=NONE) {
	   // _data should be freed, but it was used in this call...
#if DEBUG_LEVEL>=1
	   std::cerr << "Weird Image::setData call: " << std::endl ;
	   std::cerr << "  (" << s << std::hex << " 0x" << (int)d << std::dec << " " << getFreeMethodName(m) << ")" << "  --> " ;
	   debug(std::cerr) ;
	   std::cerr << std::endl ;
#endif
	   _size = s ;
	 } else {
	   _fmethod = m ;
	   _size = s ;
	 }
	 return ;
    }

    _size = s ;
    if (_fmethod) {
	 switch (_fmethod) {
	 case NONE:
	   break ;
	 case DELETE:
#if DEBUG_LEVEL>=1
	   std::cerr << "Image::setData delete [] " << std::hex << (int)_data << std::dec << std::endl ;
#endif
	   delete [] _data ;
	   _data = 0 ;
	   break ;
	 case FREE:
#if DEBUG_LEVEL>=1
	   std::cerr << "Image::setData free " << std::hex << (int)_data << std::dec << std::endl ;
#endif
	   if (_data) free(_data) ;
	   _data = 0 ;
	   break ;
	 case FREEMEM:
	   Image::FreeMem(&_data) ;
	   break ;
	 }
    }

    _data = d ;
    _fmethod = m ;
  }

  // ---------------------------------------------------------------------------

  std::string
  Image::getDescription(void) {
    std::stringstream out ;
    debug(out) ;
    return out.str() ;
  }

  void
  Image::debug(std::ostream& out) {
    if ((!_width && !_height) || _timestamp==TimeStamp::undef)
	 update_metadata(this, _encoding) ;
    out << _width << "x" << _height << " "
	   << getEncodingName(_encoding)
	   << " (" << _size 
	   << " " << std::hex << (void *)_data << std::dec 
	   << " " << getFreeMethodName(_fmethod) 
	   << ")" << " "
	   << TimeStamp::createAsStringFromInt(_timestamp) ;
  }

  // ---------------------------------------------------------------------------

  void
  Image::acquireData() {
#if DEBUG_LEVEL>=2
    std::cerr << "Image::acquireData (" << std::hex << (int)_data << std::dec << " " << _size << ")" << std::endl ;
#endif
    unsigned char *ptr = AllocMem(_size) ;
    memmove(ptr, _data, _size) ;
    setData(ptr, _size, FREEMEM) ;
  }

  // ---------------------------------------------------------------------------

  void
  Image::stealDataFrom(Image &src) {
    if (&src==this) return ;
    _timestamp = src._timestamp ;
    _width = src._width ;
    _height = src._height ;
    _encoding = src._encoding ;
    setData(src._data, src._size, src._fmethod) ;
    src._fmethod = NONE ;
  }

  void
  Image::linkDataFrom(Image &src) {
    _timestamp = src._timestamp ;
    _width = src._width ;
    _height = src._height ;
    _encoding = src._encoding ;
    setData(src._data, src._size, Image::NONE) ;
  }

  void
  Image::copyDataFrom(const Image &src) {
#if DEBUG_LEVEL>=1
    std::cerr << "Image::copyFrom " << std::hex << (int)src._data << std::dec << " (" << src._size << ") --> " ;
    std::cerr << std::hex << (int)_data << std::dec << " (" << _size << ")" << std::endl ;
#endif
    _encoding = src._encoding ;
    _timestamp = src._timestamp ;
    _width = src._width ;
    _height = src._height ;
    unsigned char *ptr = AllocMem(src._size) ;
    memmove(ptr, src._data, src._size) ;
    setData(ptr, src._size, FREEMEM) ;
  }

  // ---------------------------------------------------------------------------

  unsigned char *
  Image::AllocMem(unsigned int size) {
    // unsigned char *ptr = (unsigned char *)calloc(1, size) ;
    // unsigned char *ptr = (unsigned char *)valloc(size) ;
    unsigned char *ptr = new unsigned char [size] ;

#if DEBUG_LEVEL>=1
    std::cerr << "Image::AllocMem " ;
    std::cerr << "(" << size << ") --> " << std::hex << (int) ptr << std::dec << std::endl ;
#endif

    return ptr ;
  }

  void
  Image::FreeMem(unsigned char **thePtr) {
    unsigned char *ptr = *thePtr ;
    if (!ptr) return ;

#if DEBUG_LEVEL>=1
    std::cerr << "Image::FreeMem " ;
    std::cerr << std::hex << (int)ptr << std::dec << std::endl ;
#endif

    // free(ptr) ;
    delete [] ptr ;
    ptr = 0 ;
  }

  // ---------------------------------------------------------------------------

  void
  Image::saveAs(const std::string filename) {
    int fd = createFile(filename.c_str()) ;
    write(fd, _data, _size) ;
    close(fd) ;
  }

  // ---------------------------------------------------------------------------

  TimeStamp::inttype
  Image::getTimeStamp(void) {
    if (_timestamp==TimeStamp::undef) update_metadata(this, _encoding) ;
    return _timestamp ;
  }

  unsigned int
  Image::getWidth(void) {
    if (!_width && !_height) update_metadata(this, _encoding) ;
    return _width ;
  }

  unsigned int
  Image::getHeight(void) {
    if (!_width && !_height) update_metadata(this, _encoding) ;
    return _height ;
  }

  unsigned int
  Image::getBytesPerPixel(void) const {
    return getBytesPerPixel(_encoding) ;
  }

  bool
  Image::encodingIsConvenient(void) const {
    return encodingIsConvenient(_encoding) ;
  }

  std::string
  Image::getMimeType(void) const {
    return getEncodingMimeType(_encoding) ;
  }

}
