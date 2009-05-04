#ifndef n_Image_H
#define n_Image_H

/*
 *
 * nucleo/image/Image.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/TimeStamp.H>

#include <stdint.h>

#include <iostream>
#include <string>

namespace nucleo {

  // ------------------------------------------------------------------

#ifndef N_IMAGE_TAG
#define N_IMAGE_TAG( value, _x1, _x2, _x3, _x4 )	 \
  value = ( ( (uint32_t)_x1 << 24 ) |		 \
		  ( (uint32_t)_x2 << 16 ) |		 \
		  ( (uint32_t)_x3 << 8  ) |		 \
		  (uint32_t)_x4 )
#endif

  // ------------------------------------------------------------------

  class Image {

  public:

    typedef enum {
	 // Reserved for toolkit extensions
	 N_IMAGE_TAG(OPAQUE,'o','p','a','q'),
	 // Image source can freely choose the encoding
	 N_IMAGE_TAG(PREFERRED,'p','r','e','f'),
	 // Image source can choose any of the "convenient" encodings
	 N_IMAGE_TAG(CONVENIENT,'c','o','n','v'),
	 // -------------------------------------------------
	 // "Convenient" encodings: these images can be easily manipulated
	 N_IMAGE_TAG(L,'l','u','m','i'),
	 N_IMAGE_TAG(A,'a','l','p','h'),
	 N_IMAGE_TAG(RGB,'r','g','b',' '),
	 N_IMAGE_TAG(ARGB,'a','r','g','b'),
	 // FIXME: most filters haven't been checked for RGBA and RGB565
	 N_IMAGE_TAG(RGBA,'r','g','b','a'), 
	 N_IMAGE_TAG(RGB565,'r','g','b','5'),
	 // -------------------------------------------------
	 // Y'CbCr 4:2:0 planar: Y' components first, then Cb (1/4), then Cr (1/4)
	 N_IMAGE_TAG(YpCbCr420,'y','4','2','0'),
	 // Y'CbCr 4:2:2: (CbY0)(CrY1)
	 N_IMAGE_TAG(YpCbCr422,'y','4','2','2'),
	 // -------------------------------------------------
	 // http://www.ijg.org/
	 N_IMAGE_TAG(JPEG,'j','p','e','g'),
	 // http://www.libpng.org/pub/png/
	 N_IMAGE_TAG(PNG,'p','n','g',' '),
	 // http://netpbm.sourceforge.net/doc/pam.html
	 N_IMAGE_TAG(PAM,'p','a','m',' ')
    } Encoding ;

    static std::string getEncodingName(Encoding e) ;
    static Encoding getEncodingByName(const char *name) ;
    static Encoding getEncodingByName(std::string name) {
	 return getEncodingByName(name.c_str()) ;
    }

    static std::string getEncodingMimeType(Encoding e) ;
    static Image::Encoding getEncodingByMimeType(const char *mimetype) ;
    static Image::Encoding getEncodingByMimeType(std::string mimetype) {
	 return getEncodingByMimeType(mimetype.c_str()) ;
    }

    static unsigned int getBytesPerPixel(Encoding e) ; // 0 if not applicable
    static bool encodingIsConvenient(Encoding e) ;

    //--------------------------------------------------------------

    struct StandardSize {
	 char *name ;
	 int width, height ;
    } ;
    // The list is sorted by decreasing size and terminated by {0,0,0}
    static StandardSize StandardSizes[] ;

    //--------------------------------------------------------------

    typedef enum {
	 NONE=0,      // Memory will survive after the image
	 DELETE=1,    // Memory has to be delete'd
	 FREE=2,      // Memory has to be free'd
	 FREEMEM=4    // Memory has to be FreeMem'd
    } FreeMethod ;

    static unsigned char *AllocMem(unsigned int size) ;
    static void FreeMem(unsigned char **ptr) ;

  private:

    TimeStamp::inttype _timestamp ;
    unsigned int _width, _height ;
    Encoding _encoding ;
    unsigned char *_data ;
    unsigned int _size ;
    FreeMethod _fmethod ;

  public:

    Image(void) {
	 _timestamp = TimeStamp::undef ;
	 _width = _height = 0 ;
	 _encoding = OPAQUE ;
	 _data = 0 ;
	 _size = 0 ;
	 _fmethod = NONE ;
    }

    Image(unsigned int w, unsigned int h, Encoding e) {
	 _timestamp = TimeStamp::undef ;
	 _width = _height = 0 ;
	 _encoding = OPAQUE ;
	 _data = 0 ;
	 _size = 0 ;
	 _fmethod = NONE ;
	 prepareFor(w,h,e) ;
    }

    // Data of the new image will be *linked* to src's data
    Image(Image& src) ;              // Copy constructor
    Image& operator = (Image& src) ; // Copy assignement

    ~Image(void) { clear() ; }

    void clear(void) {
	 _timestamp = TimeStamp::undef ;
	 _width = _height = 0 ;
	 _encoding = OPAQUE ;
	 setData(0,0,NONE) ;
    }

    void prepareFor(unsigned int w, unsigned int h, Encoding e) ;

    void setTimeStamp(void) { _timestamp = TimeStamp::createAsInt() ; }
    void setTimeStamp(TimeStamp::inttype t) { _timestamp = t ; }
    void setDims(unsigned int w, unsigned int h) { _width = w ; _height = h ; }
    void setEncoding(Encoding e) { _encoding = e ; }
    void setData(unsigned char *d, unsigned int s, FreeMethod m) ;

    // src remains the data owner (this->_fmethod=NONE)
    void linkDataFrom(Image &src) ;
    // image will get src's data ownership info and src will loose
    // data ownership (src._fmethod=NONE). Warning: image might still
    // not own the data after that (dataIsLinked might help...)
    void stealDataFrom(Image &src) ;
    // This one allocates a new data buffer with AllocMem and uses
    // memmove to *copy* data from src.
    void copyDataFrom(const Image &src) ;

    bool dataIsLinked(void) { return (_fmethod==NONE) ; }

    // This one makes sure that nobody else uses the same data
    // buffer. It uses memmove to copy data from the old buffer to a
    // new one, allocated with AllocMem.
    void acquireData(void) ;

    TimeStamp::inttype getTimeStamp(void) ;

    unsigned int getWidth(void) ;
    unsigned int getHeight(void) ;

    Encoding getEncoding(void) const { return _encoding ; }
    unsigned int getBytesPerPixel(void) const ; // 0 if not applicable
    bool encodingIsConvenient(void) const ;
    std::string getMimeType(void) const ;

    unsigned char *getData(void) const { return _data ; }
    unsigned int getSize(void) const { return _size ; }

    void saveAs(const std::string filename) ;

    std::string getDescription(void) ;
    void debug(std::ostream& out) ;

  } ;

  // ------------------------------------------------------------------

}

#endif
