/*
 *
 * nucleo/core/ReactiveObject.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/ReactiveObject.H>

#include <sstream>
#include <iostream>

namespace nucleo {

  std::set<Observable*> Observable::_instances ;

  Observable::Observable(void) {
    // std::cerr << "New Observable: " << this << std::endl ;
    _instances.insert(this) ;
    _pendingNotifications = 0 ;
  }

  void
  Observable::notifyObservers(void) {
     for (std::list<ReactiveObject*>::const_iterator i=_observers.begin();
		i!=_observers.end();
		++i)
	  ReactiveEngine::notify(*i, this) ;
  }

  void
  Observable::addObserver(ReactiveObject *observer) {
    _observers.push_back(observer) ;
  }
  
  void
  Observable::removeObserver(ReactiveObject *observer) {
    _observers.remove(observer) ;
  }

  Observable::~Observable() {
    // std::cerr << "Observable " << this << " being deleted" << std::endl ;
    _instances.erase(this) ;
  }

  void
  ReactiveObject::selfNotify(void) {
    ReactiveEngine::notify(this, this) ;
  }

}
