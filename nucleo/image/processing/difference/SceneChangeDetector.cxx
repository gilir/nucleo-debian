/*
 *
 * nucleo/image/processing/difference/SceneChangeDetector.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/processing/difference/SceneChangeDetector.H>
#include <nucleo/image/processing/basic/Resize.H>

namespace nucleo {

  const char *SceneChangeDetector::statenames[] = {
    "RESET",
    "IDLE",
    "MOTION",
    "PRESENCE",
    "CONFIRMED",
    "CANCELED"
  } ;

  SceneChangeDetector::SceneChangeDetector(unsigned int scale,
								   unsigned int diffThreshold,
								   unsigned int resetTimer,
								   unsigned int presenceTimer,
								   unsigned int maxIdentical,
								   float motionThreshold,
								   float presenceThreshold,
								   float confirmedThreshold) {
    _scale = scale ;
    _diffThreshold = diffThreshold ;
    _resetTimer = resetTimer ;
    _presenceTimer = presenceTimer ;
    _maxIdentical = maxIdentical ;
    _motionThreshold = motionThreshold ;
    _presenceThreshold = presenceThreshold ;
    _confirmedThreshold = confirmedThreshold ;
    _motion = new DifferencePattern(1,1,_diffThreshold) ;
    _presence = new DifferencePattern(1,1,_diffThreshold) ;
    _confirmed = new DifferencePattern(1,1,_diffThreshold) ;
    _timer = TimeKeeper::create() ;
    setState(RESET) ;
  }

  SceneChangeDetector::~SceneChangeDetector() {
    delete _timer ;
    delete _motion ;
    delete _presence ;
    delete _confirmed ;
  }

  void
  SceneChangeDetector::setState(int s) {
    _timer->disarm() ;
    _state = s ;

    switch (_state) {
    case RESET:
	 _timer->arm(_resetTimer) ;
	 break ;
    case IDLE:
	 break ;
    case MOTION:
	 break ;
    case PRESENCE:
	 _timer->arm(_presenceTimer) ;
	 break ;
    case CONFIRMED:
	 break ;
    }
  }

  void
  SceneChangeDetector::newReferenceImage(Image *img) {
    // store the reference image in the _motion detector
    _motion->frozen = false ;
    _motion->filter(img) ;
    // subsequent differences will now use this reference image
    _motion->frozen = true ;
  }

  float
  SceneChangeDetector::motionEstimation(Image *img) {
    _motion->filter(img) ;
    float motion = (_motion->getPattern()[0])/100.0 ;
    return motion ;
  }

  float
  SceneChangeDetector::presenceEstimation(Image *img) {
    _presence->filter(img) ;
    float presence = (_presence->getPattern()[0])/100.0 ;
    return presence ;
  }

  void
  SceneChangeDetector::handle(Image *img) {
    Image mini ;
    switch (_state) {
	 // ------------------------------------------
    case RESET:
	 if (_timer->getState()==TimeKeeper::TRIGGERED) {
	   resizeImage(img, &mini, img->getWidth()/_scale, img->getHeight()/_scale) ;
	   newReferenceImage(&mini) ;
	   setState(IDLE) ;
	 }
	 break ;
	 // ------------------------------------------
    case IDLE:
	 resizeImage(img, &mini, img->getWidth()/_scale, img->getHeight()/_scale) ;
	 if(motionEstimation(&mini) > _motionThreshold) {
	   // load the image in the presence detector
	   _presence->filter(&mini) ;
	   setState(MOTION) ;
	 }
	 break ;
	 // ------------------------------------------
    case MOTION:
	 resizeImage(img, &mini, img->getWidth()/_scale, img->getHeight()/_scale) ;
	 if(motionEstimation(&mini) < _motionThreshold)
	   setState(IDLE) ;
	 else if (presenceEstimation(&mini) < _presenceThreshold)
	   setState(PRESENCE) ;
	 break ;
	 // ------------------------------------------
    case PRESENCE:
	 resizeImage(img, &mini, img->getWidth()/_scale, img->getHeight()/_scale) ;
	 if (_timer->getState()==TimeKeeper::TRIGGERED) {
	   bool newImage = true ;
	   if (_last.getWidth() && _last.getHeight()) {
		// If there was a previous "confirmed" image, compare it with
		// this one
		_confirmed->filter(&mini) ;
		float diff = (_confirmed->getPattern()[0])/100.0 ;
		if (diff<_confirmedThreshold) {
		  // Image is similar to the last one CONFIRMED
		  newImage = false ;
		  _identical++ ;
		  if (_identical>=_maxIdentical) {
		    // Too many similar images. Take the current one as the
		    // new reference image
		    _identical = 0 ;
		    newReferenceImage(&mini) ;
		  }
		  setState(CANCELED) ;
		}
	   }

	   if (newImage) {
		_last.copyDataFrom(mini) ;
		_confirmed->frozen = false ;
		_confirmed->filter(&_last) ;
		_confirmed->frozen = true ;
		_identical = 0 ;
		setState(CONFIRMED) ;
	   }
	 } else {
	   if (presenceEstimation(&mini) > _presenceThreshold) setState(MOTION) ;
	 }

	 break;
	 // ------------------------------------------
    case CONFIRMED:
	 setState(MOTION) ;
	 break ;
	 // ------------------------------------------
    case CANCELED:
	 setState(MOTION) ;
	 break ;
    } // switch
  }

}
