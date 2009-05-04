#ifndef n_TimeUtils_H
#define n_TimeUtils_H

/*
 *
 * nucleo/utils/TimeUtils.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/TimeStamp.H>

namespace nucleo {

  // ---------------------------------------------------------------

  class Chronometer {

  private:

    unsigned long _count ;
    TimeStamp::inttype _epoch, _laps ;
    bool _running ;

  public:

    Chronometer(void) { start() ; }
    
    void start(void) ; // (re)starts the chronometer
    TimeStamp::inttype read(void) ;  // number of milliseconds since last (re)start
    void stop(void) ;

    void tick(void) { _count++ ; }
    // number of ticks since last (re)start
    unsigned long count(void) { return _count ; }
    // ticks per second since last (re)start
    double average(void) { return _count/(read()/1000.0) ; }

    void reset(void) ; // stops and then clears the elapsed time and tick count

  } ;

  // ---------------------------------------------------------------

}

#endif
