#ifndef n_ImageSource_H
#define n_ImageSource_H

/*
 *
 * nucleo/image/source/ImageSource.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/URI.H>
#include <nucleo/core/ReactiveObject.H>
#include <nucleo/utils/TimeUtils.H>
#include <nucleo/image/Image.H>

namespace nucleo {

  // ------------------------------------------------------------------

  /**
   *
   * An image source should never throw any exception except in the
   * constructor. Warnings can be printed using cerr if anything goes
   * wrong. In that case, just make sure the source will never be able
   * to start.
   *
   * The constructor should set up the instance but not open any file
   * or network connection (use start() for this)
   *
   * Multiple start/.../stop should be possible...
   *
   * Whenever a new image is available, call notifyObservers
   *
   */

  class ImageSource : public ReactiveObject {

  public:

    typedef enum {STOPPED, STARTED, PAUSED} state ;

  protected:

    Image::Encoding target_encoding ;
    int64_t frameCount ;                   // number of images produced by the source
    Chronometer sampler ;                  // used to compute the mean source rate
    Image lastImage ;                      // last image produced by the source
    TimeStamp::inttype previousImageTime ;  // timestamp of the last image returned by getNextImage

  public:

    ImageSource(void) : frameCount(0), previousImageTime(TimeStamp::undef) {}
    virtual ~ImageSource(void) {}

    virtual state getState(void) = 0 ;

    bool isActive(void) {
	 static bool deprecatedWarning = true ;
	 if (deprecatedWarning) {
	   std::cerr << "Warning: ImageSource::isActive is deprecated. Use ImageSource::getState instead" << std::endl ;
	   deprecatedWarning = false ;
	 }
	 return getState()!=STOPPED ;
    }

    virtual bool start(void) = 0 ;
    int64_t getFrameCount(void) { return frameCount ; }
    virtual bool getNextImage(Image *img, TimeStamp::inttype reftime) = 0 ;
    bool getNextImage(Image *img) { return getNextImage(img, previousImageTime) ; }
    virtual bool pause(void) { return false ; }
    virtual bool resume(void) { return false ; }
    virtual bool stop(void) = 0 ;

    // Thoretical frame rate
    virtual bool setRate(double) { return false ; }
    virtual double getRate(void) { return 0.0 ; }
    // Mean (i.e. playback) frame rate
    void clearMeanRate(void) { sampler.start() ; }
    double getMeanRate(void) { return sampler.average() ; }

    virtual bool setSpeed(double) { return false ; }
    virtual double getSpeed(void) { return 1.0 ; }

    virtual TimeStamp::inttype getStartTime(void) { return TimeStamp::undef ; }
    // getDuration: result is in milliseconds
    virtual TimeStamp::inttype getDuration(void) { return TimeStamp::undef ; }
    virtual bool setTime(TimeStamp::inttype) { return false ; }

    bool waitForImage(Image *img) ;

    static ImageSource *create(const char *uri, Image::Encoding e=Image::PREFERRED) ;
    static ImageSource *create(std::string uri, Image::Encoding e=Image::PREFERRED) {
	 return create(uri.c_str(), e) ;
    }

    static bool getImage(const char *uri, Image *img, Image::Encoding enc=Image::PREFERRED) ;
    static bool getImage(std::string uri, Image *img, Image::Encoding enc=Image::PREFERRED) {
	 return getImage(uri.c_str(), img, enc) ;
    }

  } ;

  typedef ImageSource * (*ImageSourceFactory) (const URI *uri, Image::Encoding e) ;

  // ------------------------------------------------------------------

}

#endif
