#ifndef n_SceneChangeDetector_H
#define n_SceneChangeDetector_H

/*
 *
 * nucleo/image/processing/difference/SceneChangeDetector.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/processing/difference/Difference.H>
#include <nucleo/core/TimeKeeper.H>

namespace nucleo {

  class SceneChangeDetector {

  private:

    unsigned int _scale ;
    unsigned int _diffThreshold ;
    unsigned int _resetTimer ;
    unsigned int _presenceTimer ;
    unsigned int _maxIdentical ;
    float _motionThreshold ;
    float _presenceThreshold ;
    float _confirmedThreshold ;

    int _state ;
    DifferencePattern *_motion, *_presence, *_confirmed ;
    unsigned int _identical ;
    TimeKeeper *_timer ;
    Image _last ;

    void setState(int s) ;
    void newReferenceImage(Image *img) ;
    float motionEstimation(Image *img) ;
    float presenceEstimation(Image *img) ;

  public:

    enum {
	 RESET,           // Wait a few seconds, take a reference picture and switch to IDLE
	 IDLE,            // Default state
	 MOTION,          // Motion was detected
	 PRESENCE,        // Motion stopped (kind of) and the scene has changed (vs. reference picture)
	 CONFIRMED,       // Stable presence detected for more than X seconds
	 CANCELED         // Stable presence detected, but image too similar to last CONFIRMED one
    } states ;

    SceneChangeDetector(unsigned int scale=4,
				    unsigned int diffThreshold=25,
				    unsigned int resetTimer=2000, // milliseconds
				    unsigned int presenceTimer=3000, // milliseconds
				    unsigned int maxIdentical=3,
				    float motionThreshold=0.08,
				    float presenceThreshold=0.05,
				    float confirmedThreshold=0.08) ;

    ~SceneChangeDetector() ;

    static const char *statenames[] ;
    int getState(void) {
	 return _state ;
    }

    double timeRemaining(void) {
	 return (double)(_timer->getTimeLeft())/(double)_presenceTimer ;
    }

    void handle(Image *img) ;

  } ;

}

#endif
