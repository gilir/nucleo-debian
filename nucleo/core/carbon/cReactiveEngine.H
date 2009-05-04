#ifndef n_cReactiveEngine_H
#define n_cReactiveEngine_H

/*
 *
 * nucleo/core/carbon/cReactiveEngine.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>

#include <queue>
#include <Carbon/Carbon.h>

namespace nucleo {

  class cReactiveEngine : public ReactiveEngineImplementation {

  private:

    EventQueueRef _eQueue ;
    EventRef _event ;

    std::queue<ReactivePair> _objectsToNotify ;

    static OSStatus _handleEvent(EventHandlerCallRef, EventRef, void*) ;

  public:

    cReactiveEngine(void) ;    
    ~cReactiveEngine(void) ;

    void run(void) ;
    FileKeeper *createFileKeeper(void) ;
    TimeKeeper *createTimeKeeper(void) ;
    void notify(ReactiveObject *notified, Observable *notifier) ;
    void step(long milliseconds=-1) ;
    void stop(void) ;

    void wakeUp(bool force) ;

  } ;

}

#endif
