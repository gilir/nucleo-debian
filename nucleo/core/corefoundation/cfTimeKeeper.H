#ifndef n_cfTimeKeeper_H
#define n_cfTimeKeeper_H

/*
 *
 * nucleo/core/corefoundation/cfTimeKeeper.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/TimeKeeper.H>
#include <nucleo/core/TimeStamp.H>

#include <nucleo/core/carbon/cReactiveEngine.H>

#include <CoreFoundation/CoreFoundation.h>

namespace nucleo {

  // ---------------------------------------------------------------

  class cfTimeKeeper : public TimeKeeper {

  protected:

    cReactiveEngine *cEngine ;

    CFRunLoopRef _runLoop ;
    CFTimeInterval _interval ;
    CFRunLoopTimerRef _timer ;

    static void _callback(CFRunLoopTimerRef timer, void *) ;

  public:

    cfTimeKeeper(cReactiveEngine *cre=0) ;
    ~cfTimeKeeper() ;

    void arm(unsigned long milliseconds, bool repeat=false) ;
    long getTimeLeft(void) ; // in milliseconds
    void disarm(void) ;

  } ;

  // ---------------------------------------------------------------

}

#endif
