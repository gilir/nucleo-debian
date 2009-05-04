/*
 *
 * nucleo/core/TimeKeeper.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/TimeKeeper.H>

namespace nucleo {
  
  TimeKeeper *
  TimeKeeper::create(void) {
    return ReactiveEngine::createTimeKeeper() ;
  }

  TimeKeeper *
  TimeKeeper::create(unsigned long milliseconds, bool repeat) {
    TimeKeeper *tk = ReactiveEngine::createTimeKeeper() ;
    tk->arm(milliseconds, repeat) ;
    return tk ;
  }

}
