#ifndef n_MotionDetection_H
#define n_MotionDetection_H

/*
 *
 * nucleo/image/processing/difference/MotionDetection.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/Image.H>

#include <iostream>

namespace nucleo {

  // ------------------------------------------------------------------

  struct MotionArea {

    unsigned int left, top, right, bottom ;

    void clear(unsigned int x, unsigned int y) { 
	 left = right = x ;
	 top = bottom = y ;
    }

    void add(unsigned int x, unsigned int y) { 
	 if (x < left) left = x ;
	 else if (x > right) right = x ;
	 if (y < top) top = y ;
	 else if (y > bottom) bottom = y ;
    }

  } ;

  class MotionDetection {

  private:

    unsigned int _maxObject ;
    unsigned int  _nbObjects ;
    MotionArea *_objects ;

  public:

    static int MAXZONES ;

    MotionDetection(void) {
	 _maxObject = MAXZONES ;
	 _objects = new MotionArea [MAXZONES] ;
	 _nbObjects = 0 ;
    }

    void findMotionAreas(float *array, unsigned int width, unsigned int height) ;

    MotionArea *getAreas(void) { return _objects ; }
    unsigned int getNbAreas(void) { return _nbObjects ; }

    void debug(std::ostream& out) const ;

    ~MotionDetection(void) {
	 delete [] _objects ;
    }

  } ;

  // ------------------------------------------------------------------

}

#endif

