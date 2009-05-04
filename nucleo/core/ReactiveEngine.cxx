/*
 *
 * nucleo/core/ReactiveEngine.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/PluginManager.H>

#ifdef __APPLE__
#include <nucleo/core/corefoundation/cfReactiveEngine.H>
#include <nucleo/core/carbon/cReactiveEngine.H>
#endif
#include <nucleo/core/select/sReactiveEngine.H>

#include <nucleo/utils/TimeUtils.H>

#include <iostream>

namespace nucleo {

  // -----------------------------------------------------------------------

#define DEBUG_LEVEL 0

#ifdef __APPLE__
#ifdef HAVE_AGL
  std::string ReactiveEngine::defaultType = "carbon" ;
#else
  std::string ReactiveEngine::defaultType = "corefoundation" ;
#endif
#else
  std::string ReactiveEngine::defaultType = "select" ;
#endif

  std::string ReactiveEngine::engineType = ReactiveEngine::defaultType ;

  ReactiveEngineImplementation *ReactiveEngine::engine = 0 ;

  // -----------------------------------------------------------------------

  void
  ReactiveEngineImplementation::doReact(ReactiveObject*obj, Observable*obs) {
    if (ReactiveObject::isAlive(obj)) {
	 obj->react(obs) ;
    } else {
	 // std::cerr << "ReactiveEngineImplementation::doReact: " << obj << " is dead..." << std::endl ;
	 if (obs && Observable::isAlive(obs)) obs->removeObserver(obj) ;
    }
  }

  void
  ReactiveEngineImplementation::incPendingNotifications(Observable *obs, int inc) {
    if (obs && Observable::isAlive(obs)) {
	 obs->_pendingNotifications += inc ;
	 if (obs->_pendingNotifications<0) obs->_pendingNotifications = 0 ;
    }
  }	 

  void
  ReactiveEngineImplementation::sleep(long milliseconds) {
    if (milliseconds==-1)
	 ReactiveEngine::step(milliseconds) ;
    else if (milliseconds>0) {
	 TimeStamp::inttype ms=milliseconds, d=0 ;
	 Chronometer chrono ;
	 while ( (d=ms-chrono.read())>0 )
	   ReactiveEngine::step(d) ;
    }
  }

  // -----------------------------------------------------------------------
  
  std::string
  ReactiveEngine::getEngineType(void) {
    return engineType ;
  }

  bool
  ReactiveEngine::setEngineType(std::string type) {
    if (engine) return false ;
    if (type=="default") engineType = defaultType ;
    else engineType = type ;
    return true ;
  }

  ReactiveEngineImplementation*
  ReactiveEngine::getEngine(void) {
    if (engine) return engine ;
#if DEBUG_LEVEL>0
    std::cerr << "ReactiveEngine: using '" << engineType << "' implementation" << std::endl ;
#endif
#ifdef __APPLE__
    if (engineType=="carbon")
	 return (engine = (ReactiveEngineImplementation*) new cReactiveEngine) ; 
    if (engineType=="corefoundation")
	 return (engine = (ReactiveEngineImplementation*) new cfReactiveEngine) ; 
#endif
    if (engineType=="select")
	 return (engine = (ReactiveEngineImplementation*) new sReactiveEngine) ;

    // This will raise an exception if no factory can be found
    ReactiveEngineFactory factory = (ReactiveEngineFactory) PluginManager::getSymbol("ReactiveEngine::create",std::string("type=")+engineType) ;
    return (engine = (ReactiveEngineImplementation*) (*factory)()) ;
  }

  void
  ReactiveEngine::notify(ReactiveObject *notified, Observable *notifier) {
    getEngine()->notify(notified, notifier) ;
  }

  FileKeeper *
  ReactiveEngine::createFileKeeper(void) {
    return getEngine()->createFileKeeper() ;
  }

  TimeKeeper *
  ReactiveEngine::createTimeKeeper(void) {
    return getEngine()->createTimeKeeper() ;
  }

  void
  ReactiveEngine::step(long milliseconds) {
    getEngine()->step(milliseconds) ;
  }

  void
  ReactiveEngine::run(void) {
    getEngine()->run() ;
  }

  void
  ReactiveEngine::stop(void) {
    getEngine()->stop() ;
  }

  void
  ReactiveEngine::sleep(long milliseconds) {
    getEngine()->sleep(milliseconds) ;
  }

}
