/*
 *
 * nucleo/core/select/sTimeKeeper.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/core/select/sTimeKeeper.H>

#include <sys/time.h>

#include <iostream>

namespace nucleo {

  // -----------------------------------------------------------------------------

  std::list<sTimeKeeper*> sTimeKeeper::_instances ;

  sTimeKeeper::sTimeKeeper(void) {
    _instances.push_back(this) ;
  }

  sTimeKeeper::~sTimeKeeper() {
    _instances.remove(this) ;
  }

  void
  sTimeKeeper::_checkTime(void) {
    if (! (_state&ARMED)) return ; 
    TimeStamp::inttype timeleft = _epoch - TimeStamp::createAsInt() ;
    if (timeleft <= 0) {
	 _state = _repeat ? TRIGGERED_AND_ARMED : TRIGGERED ;
	 notifyObservers() ;
	 _epoch = TimeStamp::createAsInt() + _delay ;
    }
  }

  void
  sTimeKeeper::arm(unsigned long milliseconds, bool repeat) {
    _repeat = repeat ;
    _delay = milliseconds ;
    _epoch = TimeStamp::createAsInt() + milliseconds ;
    _state = ARMED ;
  }

  void
  sTimeKeeper::disarm(void) {
    _state = DISARMED ;
  }

  long
  sTimeKeeper::getTimeLeft(void) {
    if (_state==DISARMED) return -1 ;
    if (_state==TRIGGERED) return 0 ;
    TimeStamp::inttype timeleft = _epoch - TimeStamp::createAsInt() ;
    return (timeleft<0) ? 0 : timeleft ;
  }

  // -----------------------------------------------------------------------------

}
