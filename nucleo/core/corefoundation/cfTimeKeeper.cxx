/*
 *
 * nucleo/core/corefoundation/cfTimeKeeper.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/core/corefoundation/cfTimeKeeper.H>

#include <sys/time.h>

#include <iostream>

namespace nucleo {

  // -----------------------------------------------------------------------------

  void
  cfTimeKeeper::_callback(CFRunLoopTimerRef timer, void *data){
    cfTimeKeeper *obj = (cfTimeKeeper*)data ;
    // std::cerr << " [FK(" << obj << "):" << (int)obj->_observers.size() << "] " << std::flush ;

    if (obj->_observers.empty()) {
	 if (obj->cEngine) obj->cEngine->wakeUp(true) ;
    } else {
	 if (obj->_pendingNotifications<=2*(int)obj->_observers.size())
	   obj->notifyObservers() ;
#if 0
	 else
	   std::cerr << "cfTimeKeeper: " << obj << " has "  << obj->_pendingNotifications 
			   << " pending notifications" << std::endl ;
#endif
    }

    if (obj->_interval) {
	 obj->_state = TRIGGERED_AND_ARMED ;
    } else {
	 CFRelease(obj->_timer) ;
	 obj->_timer = 0 ;
	 obj->_state = TRIGGERED ;
    }
  }

  cfTimeKeeper::cfTimeKeeper(cReactiveEngine *cre) : TimeKeeper() {
    cEngine = cre ;
    _runLoop = CFRunLoopGetCurrent() ;
    // std::cerr << "cfTimeKeeper::cfTimeKeeper " << this << " (runloop: " << _runLoop << ")" << std::endl ;
    CFRetain(_runLoop) ;
    _timer = 0 ;
    _interval = 0 ;
  }

  cfTimeKeeper::~cfTimeKeeper() {
    // std::cerr << "cfTimeKeeper::~cfTimeKeeper " << this << std::endl ;
    disarm() ;
    CFRelease(_runLoop) ;
  }

  void
  cfTimeKeeper::arm(unsigned long milliseconds, bool repeat) {
    // std::cerr << "cfTimeKeeper::arm " << this << " (" << milliseconds << "," << repeat << ")" << std::endl ;
    disarm() ;

    CFRunLoopTimerContext context = {0,(void *)this,NULL,NULL,NULL};
    CFTimeInterval delta = (CFTimeInterval)milliseconds/1000.0 ;
    if (delta<=0) delta = 0.0001 ;
    _interval = repeat ? delta : 0 ;
    CFAbsoluteTime fireDate = CFAbsoluteTimeGetCurrent() + delta ;
    _timer = CFRunLoopTimerCreate(kCFAllocatorDefault,
						    fireDate, _interval, 
						    0, 0, 
						    _callback, &context) ;
    CFRunLoopAddTimer(_runLoop, _timer, kCFRunLoopCommonModes) ;

    _state = ARMED ;
  }

  long
  cfTimeKeeper::getTimeLeft(void) {
    // std::cerr << "cfTimeKeeper::getTimeLeft " << this << std::endl ;
    if (_state==DISARMED) return -1 ;
    if (_state==TRIGGERED) return 0 ;

    CFTimeInterval delta = CFRunLoopTimerGetNextFireDate(_timer) - CFAbsoluteTimeGetCurrent() ;
    return (delta<=0) ? 0 : (long)(delta*1000) ;
  }

  void
  cfTimeKeeper::disarm(void) {
    // std::cerr << "cfTimeKeeper::disarm " << this << std::endl ;
    // if (_pendingNotifications) std::cerr << "cfTimeKeeper " << this << " has " << _pendingNotifications << " notifications pending and is being disarmed" << std::endl ;
    if (_state&ARMED) {
	 if (CFRunLoopTimerIsValid(_timer)) CFRunLoopTimerInvalidate(_timer) ;
	 CFRelease(_timer) ;
	 _timer = 0 ;
    }
    _state = DISARMED ;
  }

  // -----------------------------------------------------------------------------

}
