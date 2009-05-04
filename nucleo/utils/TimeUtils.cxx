/*
 *
 * nucleo/utils/TimeUtils.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/utils/TimeUtils.H>
#include <nucleo/core/TimeStamp.H>

#include <sys/time.h>

namespace nucleo {

  // -----------------------------------------------------------------------------

  void
  Chronometer::start(void) {
    _count = 0 ;
    _running = true ;
    _laps = 0 ;
    _epoch = TimeStamp::createAsInt() ;
  }

  TimeStamp::inttype
  Chronometer::read(void) {
    if (!_running) return _laps ;
    return TimeStamp::createAsInt()-_epoch ;
  }

  void
  Chronometer::stop(void) {
    _laps = TimeStamp::createAsInt()-_epoch ;  
    _running = false ;
  }

  void
  Chronometer::reset(void) {
    stop() ;
    _laps = 0 ;
    _count = 0 ;
  }

  // -----------------------------------------------------------------------------

}
