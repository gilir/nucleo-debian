#ifndef ReactiveEngine_H
#define ReactiveEngine_H

/*
 *
 * nucleo/core/ReactiveEngine.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveObject.H>

#include <string>

namespace nucleo {

  // ----------------------------------------------------------

  class TimeKeeper ;
  class FileKeeper ;

  class ReactiveEngineImplementation {

  protected:

    void doReact(ReactiveObject*obj, Observable*obs) ;
    void incPendingNotifications(Observable *obs, int inc) ;

    ReactiveEngineImplementation(void) {}
    virtual void notify(ReactiveObject *notified, Observable *notifier) = 0 ;

    friend class ReactiveEngine ;

  public:

    virtual ~ReactiveEngineImplementation() {}

    virtual void run(void) = 0 ;

    virtual FileKeeper *createFileKeeper(void) = 0 ;
    virtual TimeKeeper *createTimeKeeper(void) = 0 ;

    // this method will run the engine for AT MOST the specified time
    // (-1 means "wait for something to happen")
    virtual void step(long milliseconds=-1) = 0 ;

    // this one will actually run the engine for the specified time
    // (again, -1 means "until something happens")
    void sleep(long milliseconds=-1) ;

    virtual void stop(void) = 0 ;

  } ;

  // ----------------------------------------------------------

  class ReactiveEngine {

  private:

    static std::string defaultType, engineType ;
    static ReactiveEngineImplementation *engine ;
    static ReactiveEngineImplementation *getEngine(void) ;

    ReactiveEngine(void) {}

  protected:

    static void notify(ReactiveObject *notified, Observable *notifier) ;

    friend class Observable ;
    friend class ReactiveObject ;

  public:

    static std::string getEngineType(void) ;
    static bool setEngineType(std::string type) ;

    static void run(void) ;
    static FileKeeper *createFileKeeper(void) ;
    static TimeKeeper *createTimeKeeper(void) ;
    static void step(long milliseconds=-1) ; 
    static void sleep(long milliseconds=-1) ;
    static void stop(void) ;

  } ;

  // ----------------------------------------------------------

  typedef ReactiveEngineImplementation* (*ReactiveEngineFactory) (void) ;

  // ----------------------------------------------------------

}

#endif
