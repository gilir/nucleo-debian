/*
 *
 * nucleo/plugins/qt/qtReactiveEngine.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/plugins/qt/qtReactiveEngine.H>
#include <nucleo/plugins/qt/qtFileKeeper.H>
#include <nucleo/plugins/qt/qtTimeKeeper.H>

#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QTimer>

#include <iostream>

extern "C" {
  void *qtReactiveEngine_factory(void) {
    // std::cerr << "qtReactiveEngine_factory" << std::endl ;
    return (void *)(new nucleo::qtReactiveEngine) ;
  }
}

namespace nucleo {

#define NUCLEO_CUSTOM_EVENT (QEvent::Type)(QEvent::User+1971)

  class qtNucleoEvent : public QEvent {
  public:
    ReactiveObject *notified ;
    Observable *notifier ;
    qtNucleoEvent(ReactiveObject *r, Observable *o) : QEvent(NUCLEO_CUSTOM_EVENT) {
	 notified = r ;
	 notifier = o ;
    }
  } ;


  // ---------------------------------------------------------------------

#if USE_QT_HELPER
  void
  qtReactiveEngineHelper::customEvent(QEvent *event) {
#else
  void
  qtReactiveEngine::customEvent(QEvent *event) {
#endif
    if (event->type()!=NUCLEO_CUSTOM_EVENT) {
	 std::cerr << "qtReactiveEngine::customEvent: unknown event type " 
			 << event->type() << std::endl ;
	 QObject::customEvent(event) ;
	 // event->ignore() ;
	 return ;
    }

    event->accept() ;
    qtNucleoEvent *e = (qtNucleoEvent*)event ;
    // std::cerr << "qtReactiveEngine::customEvent: " << e->observable << " -> " << e->observer << std::endl ;
#if USE_QT_HELPER
    master->incPendingNotifications(e->notifier, -1) ;
    master->doReact(e->notified, e->notifier) ;
#else
    incPendingNotifications(e->notifier, -1) ;
    doReact(e->notified, e->notifier) ;
#endif
  }

  // ---------------------------------------------------------------------

  qtReactiveEngine::qtReactiveEngine(void) {
    QCoreApplication *app = QCoreApplication::instance() ;
    if (!app) {
	 std::cerr << "qtReactiveEngine: no QCoreApplication, creating one" << std::endl ;
	 int argc = 0 ; char *argv[] = {""} ;
	 app = new QCoreApplication(argc,(char**)argv) ;
    } else {
	 // std::cerr << "qtReactiveEngine: found a QCoreApplication!" << std::endl ;
    }
#if USE_QT_HELPER
    helper = new qtReactiveEngineHelper(this) ;
#endif
  }

  qtReactiveEngine::~qtReactiveEngine(void) {
#if USE_QT_HELPER
    delete helper ;
#endif
  }

  void
  qtReactiveEngine::run(void) {
    // std::cerr << "qtReactiveEngine::run" << std::endl ;
    QCoreApplication::exec() ;
  }

  FileKeeper *
  qtReactiveEngine::createFileKeeper(void) {
    // std::cerr << "qtReactiveEngine::createFileKeeper" << std::endl ;
    return new qtFileKeeper ;
  }

  TimeKeeper *
  qtReactiveEngine::createTimeKeeper(void) {
    // std::cerr << "qtReactiveEngine::createTimeKeeper" << std::endl ;
    return new qtTimeKeeper ;
  }

  void
  qtReactiveEngine::notify(ReactiveObject *notified, Observable *notifier) {
    // std::cerr << "qtReactiveEngine::notify: " << notifier << " --> " << notified << std::endl ;
    incPendingNotifications(notifier, +1) ;
#if USE_QT_HELPER
    QCoreApplication::postEvent((QObject*)helper, new qtNucleoEvent(notified,notifier)) ;
#else
    QCoreApplication::postEvent((QObject*)this, new qtNucleoEvent(notified,notifier)) ;
#endif
  }

  void
  qtReactiveEngine::step(long milliseconds) {
    QTimer timer ;
    if (milliseconds>=0) {
	 timer.setSingleShot(true) ;
	 timer.start(milliseconds) ;
    }
    QAbstractEventDispatcher *disp = QAbstractEventDispatcher::instance() ;
    /* bool processed = */ disp->processEvents(QEventLoop::AllEvents|QEventLoop::WaitForMoreEvents) ;
  }

  void
  qtReactiveEngine::stop(void) {
    // std::cerr << "qtReactiveEngine::stop" << std::endl ;
    QCoreApplication::exit() ;
  }

}
