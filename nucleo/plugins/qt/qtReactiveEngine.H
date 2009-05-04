#ifndef n_qtReactiveEngine_H
#define n_qtReactiveEngine_H

/*
 *
 * nucleo/plugins/qt/qtReactiveEngine.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>

#include <QObject>

namespace nucleo {

  // FIXME: I can't understand why qtReactiveEngine can't inherit both
  // from QObject and ReactiveEngineImplementation (segfaults...)
#define USE_QT_HELPER 1

#if USE_QT_HELPER
  class qtReactiveEngine ;

  class qtReactiveEngineHelper : public QObject {

  Q_OBJECT

  protected:

    qtReactiveEngine *master ;

    void customEvent(QEvent *event) ;

  public:

    qtReactiveEngineHelper(qtReactiveEngine *m) : master(m) {}

  } ;
#endif

#if USE_QT_HELPER
  class qtReactiveEngine : public ReactiveEngineImplementation {
#else
    class qtReactiveEngine : public QObject, public ReactiveEngineImplementation {
    Q_OBJECT
#endif

  protected:

#if USE_QT_HELPER
    qtReactiveEngineHelper *helper ;
    friend class qtReactiveEngineHelper ;
#else
    void customEvent(QEvent *event) ;
#endif

  public:

    qtReactiveEngine(void) ;
    ~qtReactiveEngine(void) ;
  
    void run(void) ;
    FileKeeper *createFileKeeper(void) ;
    TimeKeeper *createTimeKeeper(void) ;
    void notify(ReactiveObject *notified, Observable *notifier) ;
    void step(long milliseconds=-1) ;
    void stop(void) ;
  
  } ;

}

#endif
