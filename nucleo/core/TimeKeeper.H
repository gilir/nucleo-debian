#ifndef n_TimeKeeper_H
#define n_TimeKeeper_H

/*
 *
 * nucleo/core/TimeKeeper.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>

namespace nucleo {

  // ---------------------------------------------------------------

  class TimeKeeper : public Observable {

  public:

    typedef enum {DISARMED=0, ARMED=1, TRIGGERED=2, TRIGGERED_AND_ARMED=3} state ;

  protected:

    state _state ;

  public:

    enum {millisecond=1, second=1000, minute=60000} units ;

    TimeKeeper() : _state(DISARMED) {}
    virtual ~TimeKeeper() {}

    virtual void arm(unsigned long milliseconds, bool repeat=false) = 0 ;
    virtual long getTimeLeft(void) = 0 ; // in milliseconds
    virtual void disarm(void) = 0 ;

    state getState(void) { return _state ; }

    static TimeKeeper *create(void) ;
    static TimeKeeper *create(unsigned long milliseconds, bool repeat=false) ;

  } ;

  // ---------------------------------------------------------------

}

#endif
