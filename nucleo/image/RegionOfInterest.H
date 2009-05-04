#ifndef n_RegionOfInterest_H
#define n_RegionOfInterest_H

/*
 *
 * nucleo/image/RegionOfInterest.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/Image.H>

#include <sstream>

namespace nucleo {

  // ------------------------------------------------------------------

  struct RegionOfInterest {
    int minx, miny, maxx, maxy ;

    RegionOfInterest(int maxval=32767) {
	 minx = miny = maxval ;
	 maxx = maxy = 0 ;
    }

    RegionOfInterest(int x1, int y1, int x2, int y2) {
	 minx = x1 ; miny = y1 ;
	 maxx = x2 ; maxy = y2 ;
    }

    RegionOfInterest(Image &image) {
	 minx = miny = 0 ;
	 maxx = image.getWidth()-1 ;
	 maxy = image.getHeight()-1 ;
    }

    RegionOfInterest& operator = (RegionOfInterest& other) {
	 minx = other.minx ; miny = other.miny ;
	 maxx = other.maxx ; maxy = other.maxy ;
	 return *this ;
    }

    void set(int x1, int y1, int x2, int y2) {
	 minx = x1 ; miny = y1 ;
	 maxx = x2 ; maxy = y2 ;
    }

    void grow(RegionOfInterest &other) {
	 grow(other.minx,other.miny,other.maxx,other.maxy) ;
    }
    void grow(int x1, int y1, int x2, int y2) {
	 if (minx>x1) minx = x1 ; if (miny>y1) miny = y1 ;
	 if (maxx<x2) maxx = x2 ; if (maxy<y2) maxy = y2 ;
    }

    void constrain(RegionOfInterest &other) {
	 constrain(other.minx,other.miny,other.maxx,other.maxy) ;
    }
    void constrain(int x1, int y1, int x2, int y2) {
	 if (minx<x1) minx = x1 ; if (miny<y1) miny = y1 ;
	 if (maxx>x2) maxx = x2 ; if (maxy>y2) maxy = y2 ;
    }

    void slide(int dx, int dy) {
	 minx+=dx ; maxx+=dx ;
	 miny+=dy ; maxy+=dy ; 
    }

    void expand(int offset) {
	 minx-=offset ; miny-=offset ;
	 maxx+=offset ; maxy+=offset ;
    }

    std::string asString(void) {
	 std::stringstream tmp ;
	 tmp << "[" << minx << "," << miny << " - " << maxx << "," << maxy << "]" ;
	 return tmp.str() ;
    }

  } ;

  // ------------------------------------------------------------------

}

#endif
