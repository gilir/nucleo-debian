/*
 *
 * nucleo/core/carbon/cReactiveEngine.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/core/cocoa/cPoolTrick.H>

#include <nucleo/core/carbon/cReactiveEngine.H>
#include <nucleo/core/corefoundation/cfTimeKeeper.H>
#include <nucleo/core/corefoundation/cfFileKeeper.H>

#include <Carbon/Carbon.h>

#include <iostream>

namespace nucleo {

  static const long cReactiveEngine_event_class = 'nucE' ;
  static const long cReactiveEngine_event_kind = 'noti' ;

  // ----------------------------------------------------------

  cReactiveEngine::cReactiveEngine(void) {
    EventTypeSpec ets ;
    ets.eventClass = cReactiveEngine_event_class ;
    ets.eventKind = cReactiveEngine_event_kind ;
    InstallEventHandler(GetApplicationEventTarget(),
				    NewEventHandlerUPP(_handleEvent),
				    1, &ets,
				    (void *)this, NULL /* outRef */) ;
    _eQueue = GetCurrentEventQueue() ;
    CreateEvent(0, cReactiveEngine_event_class, cReactiveEngine_event_kind,
			 0, kEventAttributeUserEvent, &_event) ;

    doThePoolTrick() ;
  }

  cReactiveEngine::~cReactiveEngine(void) {
    ReleaseEvent(_event) ;
  }

  // ----------------------------------------------------------

  FileKeeper *
  cReactiveEngine::createFileKeeper(void) {
    return new cfFileKeeper(this) ;
  }
   
  TimeKeeper *
  cReactiveEngine::createTimeKeeper(void) {
    return new cfTimeKeeper(this) ;
  }

  void
  cReactiveEngine::wakeUp(bool force) {
    // std::cerr << " [wakeUp" ;
    if (force || !_objectsToNotify.empty()) {
	 // std::cerr << "!" ;
#if 0
	 EventRef event = 0 ;
	 CreateEvent(0, cReactiveEngine_event_class, cReactiveEngine_event_kind,
			   0, kEventAttributeUserEvent, &event) ;
	 PostEventToQueue(_eQueue, event, kEventPriorityLow) ;
	 ReleaseEvent(event) ;
#else
	 PostEventToQueue(_eQueue, _event, kEventPriorityLow) ;
#endif
    }
    // std::cerr << "] " << std::flush ;
  }

  // ----------------------------------------------------------

  OSStatus
  cReactiveEngine::_handleEvent(EventHandlerCallRef nextHandler,
						  EventRef theEvent, void* userData) {
    cReactiveEngine *myself = (cReactiveEngine*)userData ;

    for (unsigned int n=myself->_objectsToNotify.size();
	    !myself->_objectsToNotify.empty() && n; --n) {
	 ReactivePair pair = myself->_objectsToNotify.front() ; 
	 myself->_objectsToNotify.pop() ;
	 myself->incPendingNotifications(pair.second, -1) ;
	 myself->doReact(pair.first, pair.second) ;
    }

    myself->wakeUp(false) ;
    return noErr ;
  }

  void
  cReactiveEngine::notify(ReactiveObject *notified, Observable *notifier) {
    bool wakeMeUp = _objectsToNotify.empty() ;

    incPendingNotifications(notifier, +1) ;   
    _objectsToNotify.push(ReactivePair(notified,notifier)) ;

    if (wakeMeUp) wakeUp(true) ;
  }

  // ----------------------------------------------------------

  void
  cReactiveEngine::step(long milliseconds) {
    EventRef theEvent ;
    EventTimeout carbon_timeout = (milliseconds==-1) ? kEventDurationForever : milliseconds*kEventDurationMillisecond ;
    if (ReceiveNextEvent(0, NULL, carbon_timeout, true, &theEvent)!=noErr) return ;

    SendEventToEventTarget(theEvent, GetEventDispatcherTarget()) ;
    ReleaseEvent(theEvent) ;
  }

  void
  cReactiveEngine::run(void) {
    RunApplicationEventLoop() ;
  }

  void
  cReactiveEngine::stop(void) {
    QuitApplicationEventLoop() ;
  }

}
