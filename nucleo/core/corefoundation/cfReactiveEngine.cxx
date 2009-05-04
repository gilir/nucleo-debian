#include <nucleo/config.H>

#include <nucleo/core/corefoundation/cfReactiveEngine.H>
#include <nucleo/core/corefoundation/cfFileKeeper.H>
#include <nucleo/core/corefoundation/cfTimeKeeper.H>

#include <iostream>

namespace nucleo {

  void
  cfReactiveEngine::callback(CFRunLoopObserverRef observer, CFRunLoopActivity activity,
					   void *info) {
    cfReactiveEngine *myself = (cfReactiveEngine*)info ;

    unsigned int nb2notify = myself->_objectsToNotify.size() ;
    if (!nb2notify) return ;

    // std::cerr << "cfReactiveEngine::callback: handling " << nb2notify << " notifications" << std::endl ;
    for (unsigned int i=0; !myself->_objectsToNotify.empty() && i<nb2notify; ++i) {
	 ReactivePair pair = myself->_objectsToNotify.front() ; 
	 myself->_objectsToNotify.pop() ;
	 myself->incPendingNotifications(pair.second, -1) ;
	 myself->doReact(pair.first, pair.second) ;
    }
  }

  cfReactiveEngine::cfReactiveEngine(void) {
    _runLoop = CFRunLoopGetCurrent() ;

    CFRunLoopObserverContext context = {0, (void*)this, 0,0,0} ;
    _observer = CFRunLoopObserverCreate(kCFAllocatorDefault,
								kCFRunLoopAllActivities
								/*kCFRunLoopBeforeWaiting|kCFRunLoopAfterWaiting*/,
								true, 0, 
								callback, &context) ;

    CFRunLoopAddObserver(_runLoop, _observer, kCFRunLoopCommonModes) ;
  }

  cfReactiveEngine::~cfReactiveEngine(void) {
    // FIXME: empty _objectsToNotify
  }

  FileKeeper *
  cfReactiveEngine::createFileKeeper(void) {
    return new cfFileKeeper ;
  }
   
  TimeKeeper *
  cfReactiveEngine::createTimeKeeper(void) {
    return new cfTimeKeeper ;
  }

  void
  cfReactiveEngine::notify(ReactiveObject *notified, Observable *notifier) {
    // std::cerr << "cfReactiveEngine::notify " << notifier << " --> " << notified << std::endl ;
    incPendingNotifications(notifier, +1) ;   
    _objectsToNotify.push(ReactivePair(notified,notifier)) ;
  }

  void
  cfReactiveEngine::step(long milliseconds) {
    SInt32 result = 0 ;
    do {
	 CFTimeInterval seconds = 3600.0 ; // one hour
	 if (milliseconds>=0) seconds = (CFTimeInterval)milliseconds/1000.0 ;
	 result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, true) ;
	 // std::cerr << "CFRunLoopRunInMode: " << result << std::endl ;
    } while (milliseconds==-1 && result==kCFRunLoopRunTimedOut) ;
  }

  void
  cfReactiveEngine::run(void) {
    CFRunLoopRun() ;
  }

  void
  cfReactiveEngine::stop(void) {
    CFRunLoopStop(CFRunLoopGetCurrent()) ;
  }

}
