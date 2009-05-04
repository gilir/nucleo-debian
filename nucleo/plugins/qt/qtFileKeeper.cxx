/*
 *
 * nucleo/plugins/qt/qtFileKeeper.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/plugins/qt/qtFileKeeper.H>

#include <iostream>

namespace nucleo {

  qtFileKeeper::qtFileKeeper(void) {
    rsn = wsn = esn = 0 ;
  }

  qtFileKeeper::~qtFileKeeper(void) {
    cleanup() ;
  }

  void
  qtFileKeeper::cleanup(void) {
    if (rsn) { delete rsn ; rsn = 0 ; }
    if (wsn) { delete wsn ; wsn = 0 ; }
    if (esn) { delete esn ; esn = 0 ; }
    _fd = -1 ;
    _mask = NOTHING ;
  }

  void
  qtFileKeeper::setup(int fd, eventMask mask) {
    cleanup() ;
    FileKeeper::setup(fd, mask) ;

    if (mask&FileKeeper::R) {
	 rsn = new QSocketNotifier(fd, QSocketNotifier::Read) ;
	 QObject::connect(rsn, SIGNAL(activated(int)), this, SLOT(callback())) ;
	 rsn->setEnabled(true) ;
    }
    if (mask&FileKeeper::W) {
	 wsn = new QSocketNotifier(fd, QSocketNotifier::Write) ;
	 QObject::connect(wsn, SIGNAL(activated(int)), this, SLOT(callback())) ;
	 wsn->setEnabled(true) ;
    }
    if (mask&FileKeeper::E) {
	 esn = new QSocketNotifier(fd, QSocketNotifier::Exception) ;
	 QObject::connect(esn, SIGNAL(activated(int)), this, SLOT(callback())) ;
	 esn->setEnabled(true) ;
    }
  }

  void
  qtFileKeeper::callback(void) {
    // std::cerr << "callback!" << std::endl ;
    if (_pendingNotifications<=2*(int)_observers.size())
	 notifyObservers() ;
  }

}
