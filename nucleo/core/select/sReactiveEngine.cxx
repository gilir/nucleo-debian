/*
 *
 * nucleo/core/select/sReactiveEngine.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/core/select/sReactiveEngine.H>
#include <nucleo/core/select/sTimeKeeper.H>
#include <nucleo/core/select/sFileKeeper.H>

#include <sys/select.h>

#include <iostream>

namespace nucleo {

  sReactiveEngine::sReactiveEngine(void) {
  }

  sReactiveEngine::~sReactiveEngine(void) {
  }

  void
  sReactiveEngine::notify(ReactiveObject *notified, Observable *notifier) {
    _objectsToNotify.push(ReactivePair(notified,notifier)) ;
    incPendingNotifications(notifier, +1) ;
  }

  void
  sReactiveEngine::step(long timeout) {
    bool timedout = false ;
    int nbEvents = 0 ;

    sTimeKeeper *timeout_tk = 0 ;
    if (timeout!=-1) {
	 timeout_tk = new sTimeKeeper() ;
	 timeout_tk->arm(timeout) ;
    }

    while (!timedout && !nbEvents) {

	 // ------------------------------------------------

	 int maxFd = -1 ;
	 fd_set rfds, wfds, efds ;
	 FD_ZERO(&rfds) ;
	 FD_ZERO(&wfds) ;
	 FD_ZERO(&efds) ;

	 // std::cerr << "FileKeepers: " << std::flush ;
	 for (std::list<sFileKeeper*>::const_iterator i=sFileKeeper::_instances.begin();
		 i!=sFileKeeper::_instances.end();
		 ++i) {
	   sFileKeeper *fk = (*i) ;
	   int fd = fk->getFd() ;
	   // std::cerr << fk << "(" << fd << ") " << std::flush ;
	   if (fd>-1) {
		int events = fk->getEventMask() ;
		if (fd>maxFd) maxFd = fd ;
		if (events&FileKeeper::R) FD_SET(fd, &rfds) ;
		if (events&FileKeeper::W) FD_SET(fd, &wfds) ;
		if (events&FileKeeper::E) FD_SET(fd, &efds) ;
	   }
	 }
	 // std::cerr << std::endl ;
 
	 long timeout = _objectsToNotify.empty() ? -1 : 0 ;
	 if (timeout!=0)
	   for (std::list<sTimeKeeper*>::const_iterator i=sTimeKeeper::_instances.begin();
		   i!=sTimeKeeper::_instances.end();
		   ++i) {
		sTimeKeeper *tk = (*i) ;
		if (tk->getState()&sTimeKeeper::ARMED) {
		  long milliseconds = tk->getTimeLeft() ;
		  if (timeout<0 || (milliseconds>=0 && timeout>milliseconds))
		    timeout = milliseconds ;
		}
	   }

	 // ------------------------------------------------
	 // select

	 if (timeout!=0 || maxFd!=-1) {
	   //    std::cerr << "sReactiveEngine::step timeout=" << timeout << " maxFd=" << maxFd ;
	   if (timeout>-1) {
		struct timeval tv ;
		tv.tv_sec = timeout/1000 ;
		tv.tv_usec = (timeout%1000)*1000 ;
		if (select(maxFd+1,&rfds,&wfds,&efds,&tv)==-1) {
		  delete timeout_tk ;
		  return ;
		}
	   } else {
		if (select(maxFd+1,&rfds,&wfds,&efds,0)==-1) {
		  delete timeout_tk ;
		  return ;
		}
	   }
	   //    std::cerr << std::endl ;
	 }

	 if (maxFd>=0)
	   for (std::list<sFileKeeper*>::const_iterator i=sFileKeeper::_instances.begin();
		   i!=sFileKeeper::_instances.end();
		   ++i) {
		sFileKeeper *fk = (*i) ;
		int fd = fk->getFd() ;
		if (fd>-1) {
		  int state = FileKeeper::NOTHING ;
		  if (FD_ISSET(fd, &rfds)) state = state|FileKeeper::R ;
		  if (FD_ISSET(fd, &wfds)) state = state|FileKeeper::W ;
		  if (FD_ISSET(fd, &efds)) state = state|FileKeeper::E ;
		  if (state&fk->getEventMask()) {
		    fk->notifyObservers() ;
		    nbEvents++ ;
		  }
		}
	   }

	 for (std::list<sTimeKeeper*>::const_iterator i=sTimeKeeper::_instances.begin();
		 i!=sTimeKeeper::_instances.end();
		 ++i) {
	   sTimeKeeper *tk = (*i) ;
	   tk->_checkTime() ;
	   if (tk->getState()&sTimeKeeper::TRIGGERED) timedout = true ;
	 }

	 // ------------------------------------------------

	 unsigned int nb2notify = _objectsToNotify.size() ;
	 if (nb2notify) {
	   nbEvents++ ;
	   for (unsigned int i=0; i<nb2notify; ++i) {
		ReactivePair pair = _objectsToNotify.front() ; 
		_objectsToNotify.pop() ;
		incPendingNotifications(pair.second, -1) ;
		doReact(pair.first, pair.second) ;
	   }
	 }

    }

    delete timeout_tk ;
  }

  void
  sReactiveEngine::run(void) {
    for (_running=true; _running;) step() ;
  }

  FileKeeper *
  sReactiveEngine::createFileKeeper(void) {
    return new sFileKeeper ;
  }

  TimeKeeper *
  sReactiveEngine::createTimeKeeper(void) {
    return new sTimeKeeper ;
  }

  void
  sReactiveEngine::stop(void) {
    _running = false ;
  }

}
