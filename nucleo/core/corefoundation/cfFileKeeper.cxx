/*
 *
 * nucleo/core/corefoundation/cfFileKeeper.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/core/corefoundation/cfFileKeeper.H>
#include <nucleo/utils/FileUtils.H>

#include <sys/types.h>
#include <sys/socket.h>

#include <iostream>

namespace nucleo {
  
  /*
    Problem #1: The CF callback mechanism doesn't give any clue of the
    current __state__ of the socket (if I receive a
    kCFSocketReadCallBack, how do I know whether it's also writable?).
    
    Solution to problem #1: use FileKeeper::getState

    Problem #2: We sometimes get notified by CF that a TCP socket is
    readable although there is no data available (the reason for this
    is that when CF notifies us, we don't read data but generate a
    nucleo notification which is handled later, possibly after another
    CF notification).

    Solution to problem #2: This is not really a problem as long as
    ReactiveObjects check the current state of the FileKeeper before
    reacting.
  */

  // -----------------------------------------------------------------------------

  void
  cfFileKeeper::_callback(CFSocketRef s,
					CFSocketCallBackType type,
					CFDataRef address, const void *data, void *info) {
    cfFileKeeper *obj = (cfFileKeeper *)info ;
    // std::cerr << " [TK(" << obj << "):" << (int)obj->_observers.size() << "] " << std::flush ;

    if (obj->_observers.empty()) {
	 if (obj->cEngine) obj->cEngine->wakeUp(true) ;
    } else {
	 if (obj->_pendingNotifications<=2*(int)obj->_observers.size())
	   obj->notifyObservers() ;
#if 0
	 else
	   std::cerr << "cfFileKeeper: " << obj  << " has " << obj->_pendingNotifications
			   << " pending notifications" << std::endl ;
#endif
    }
    CFSocketEnableCallBacks(s, type) ;
  }

  cfFileKeeper::cfFileKeeper(cReactiveEngine *cre) {
    cEngine = cre ;
    _runLoop = CFRunLoopGetCurrent() ;
    // std::cerr << "cfFileKeeper::cfFileKeeper " << this << " (runloop: " << _runLoop << ")" << std::endl ;
    CFRetain(_runLoop) ;

    _socket = 0 ;
    _runLoopSource = 0 ;
  }

  cfFileKeeper::~cfFileKeeper(void) {
    // std::cerr << "cfFileKeeper::~cfFileKeeper " << this << std::endl ;
    _cleanup() ;
    CFRelease(_runLoop) ;
  }

  void
  cfFileKeeper::_cleanup(void) {
    // std::cerr << "cfFileKeeper::_cleanup " << this << std::endl ;

    if (_runLoopSource) {    
	 CFRunLoopRemoveSource(_runLoop, _runLoopSource, kCFRunLoopCommonModes);
	 CFRelease(_runLoopSource);
	 _runLoopSource = 0 ;
    }

    if (_socket) {
	 CFSocketInvalidate(_socket);
	 CFRelease(_socket);
	 _socket = 0 ;
    }

    _fd = -1 ;
    _mask = NOTHING ;
  }

  void
  cfFileKeeper::setup(int fd, eventMask mask) {
    // std::cerr << "cfFileKeeper::setup " << this << " (" << fd << "," << mask << ")" << std::endl ;
    _cleanup() ;

    FileKeeper::setup(fd, mask) ;

    CFOptionFlags flags = kCFSocketNoCallBack ;
    if (mask&FileKeeper::R) flags = flags | kCFSocketReadCallBack ;
    if (mask&FileKeeper::W) flags = flags | kCFSocketWriteCallBack ;
    CFSocketContext context={0,(void *)this,NULL,NULL,NULL};
    _socket = CFSocketCreateWithNative(NULL, fd, flags, _callback, &context) ;
    if (!_socket) { _cleanup() ; return ; }
    CFSocketSetSocketFlags(_socket, 0) ; // turn off automatic callback re-enabling

    _runLoopSource = CFSocketCreateRunLoopSource(NULL, _socket, 0);
    if (!_runLoopSource) { _cleanup() ; return ; }
    CFRunLoopAddSource(_runLoop, _runLoopSource, kCFRunLoopDefaultMode) ;
  }

  // -----------------------------------------------------------------------------

}
