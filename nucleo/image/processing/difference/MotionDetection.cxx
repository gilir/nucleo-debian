/*
 *
 * nucleo/image/processing/difference/MotionDetection.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/processing/difference/MotionDetection.H>

static void
explore(float *current, int x, int y,
	   int width, int height,
	   nucleo::MotionArea &o) {
  if ( *current ) {
    o.add(x, y);
    *current = 0;
    if (x > 0)
	 explore(current - 1, x - 1, y, width, height, o);
    if (x < width-1)
	 explore(current + 1, x + 1, y, width, height, o);
    if (y > 0)
	 explore(current - width, x, y - 1, width, height, o);
    if (y < height-1)
	 explore(current + width, x, y + 1, width, height, o);
  }
}

namespace nucleo {

  // ------------------------------------------------------------------

  int MotionDetection::MAXZONES = 124 ;


  // Input:  an array of char; each char nothing=0, something!=0;
  //         width, height: the size of the array
  // Output: change objectList and numObjects
  // array is cleared to zero

  void
  MotionDetection::findMotionAreas(float *array,
							unsigned int width, unsigned int height) {
    float *p = array;
    unsigned n = width * height;

    _nbObjects = 0 ;

    for(; n; n--, ++p)
	 if (*p) {
	   if (_nbObjects==_maxObject) {
		std::cerr << __FILE__ << " (" << __LINE__ << ") Maximum number of visible objects reached !" << std::endl ;
		return ;
	   }
	   MotionArea *current = _objects + _nbObjects ;
	   unsigned int pos = p - array ;
	   unsigned int x = pos % width ;
	   unsigned int y = pos / width ;
	   current->clear(x, y);
	   explore(p, x, y, width, height, _objects[_nbObjects]);
	   ++_nbObjects;
	 }
  }

  void
  MotionDetection::debug(std::ostream& out) const {
    for (unsigned int i=0; i<_nbObjects; ++i) {
	 out << "Object #" << i << " " ;
	 out << _objects[i].left << "," << _objects[i].top ;
	 out << " " ;
	 out << _objects[i].right << "," << _objects[i].bottom ;
	 out << std::endl ;
    }
  }

  // ------------------------------------------------------------------

}
