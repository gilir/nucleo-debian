#ifndef cfReactiveEngine_H
#define cfReactiveEngine_H

#include <nucleo/core/ReactiveEngine.H>

#include <CoreFoundation/CoreFoundation.h>

#include <queue>

namespace nucleo {

  class cfReactiveEngine : public ReactiveEngineImplementation {

  private:

    CFRunLoopRef _runLoop  ;
    CFRunLoopObserverRef _observer ;
    std::queue<ReactivePair> _objectsToNotify ;

    static void callback(CFRunLoopObserverRef observer, CFRunLoopActivity activity,
					void *info) ;

  public:

    cfReactiveEngine(void) ;    
    ~cfReactiveEngine(void) ;

    void run(void) ;
    FileKeeper *createFileKeeper(void) ;
    TimeKeeper *createTimeKeeper(void) ;
    void notify(ReactiveObject *notified, Observable *notifier) ;
    void step(long milliseconds=-1) ;
    void stop(void) ;

  } ;

}

#endif

