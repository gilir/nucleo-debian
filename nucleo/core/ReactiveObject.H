#ifndef n_ReactiveObject_H
#define n_ReactiveObject_H

/*
 *
 * nucleo/core/ReactiveObject.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <set>
#include <list>
#include <string>
#include <utility>

namespace nucleo {

  class ReactiveObject ;

  // ---------------------------------------------------------

  class Observable {

  private:

    static std::set<Observable*> _instances ;

  protected:

    int _pendingNotifications ;
    std::list<ReactiveObject*> _observers ;

    void notifyObservers(void) ;

    friend class ReactiveEngineImplementation ;

  public:

    Observable(void) ;

    static bool isAlive(Observable *o) {
	 return (_instances.find(o)!=_instances.end()) ;
    }

    void addObserver(ReactiveObject *observer) ;
    void removeObserver(ReactiveObject *observer) ;

    virtual ~Observable() ;

  } ;

  // ---------------------------------------------------------

  class ReactiveObject : public Observable {

  protected:

    void selfNotify(void) ;
    virtual void react(Observable*) {}

    friend class ReactiveEngineImplementation ;

  public:

    void subscribeTo(Observable &o) { o.addObserver(this) ; }
    void subscribeTo(Observable *o) { if (o && isAlive(o)) o->addObserver(this) ; }

    void unsubscribeFrom(Observable &o) { o.removeObserver(this) ; }
    void unsubscribeFrom(Observable *o) { if (o && isAlive(o)) o->removeObserver(this) ; }

  } ;

  // ---------------------------------------------------------

  class WatchDog : public ReactiveObject {

  protected:

    Observable *obs ;
    bool notified ;

    void react(Observable *o) {
	 if (obs==o) notified = true ;
    }

  public:

    WatchDog(Observable *o) : obs(o), notified(false) {
	 subscribeTo(o) ;
    }

    ~WatchDog(void) {
	 if (Observable::isAlive(obs))
	   unsubscribeFrom(obs) ;
    }

    bool sawSomething(void) {
	 bool result = notified ;
	 notified = false ; 
	 return result ;
    }

  } ;

  // ---------------------------------------------------------

  typedef std::pair<ReactiveObject*,Observable*> ReactivePair ;

}

#endif
