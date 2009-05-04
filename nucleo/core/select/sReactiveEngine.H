#ifndef n_sReactiveEngine_H
#define n_sReactiveEngine_H

/*
 *
 * nucleo/core/select/sReactiveEngine.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>

#include <queue>

namespace nucleo {

  class sReactiveEngine : public ReactiveEngineImplementation {

  private:

    bool _running ;

    std::queue<ReactivePair> _objectsToNotify ;

  public:

    sReactiveEngine(void) ;
    ~sReactiveEngine(void) ;

    void run(void) ;
    FileKeeper *createFileKeeper(void) ;
    TimeKeeper *createTimeKeeper(void) ;
    void notify(ReactiveObject *notified, Observable *notifier) ;
    void step(long milliseconds=-1) ;
    void stop(void) ;

  } ;

}

#endif
