/*
 *
 * nucleo/plugins/qt/qtTimeKeeper.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/plugins/qt/qtTimeKeeper.H>

#include <iostream>

namespace nucleo {

  void
  qtTimeKeeper::timerEvent(QTimerEvent *event) {
    // std::cerr << "qtTimeKeeper::timerEvent" << std::endl ;
    triggertime = TimeStamp::createAsInt() ;
    notifyObservers() ;
    if (repeat) {
	 _state = TRIGGERED_AND_ARMED ;
    } else {
	 _state = TRIGGERED ;
	 killTimer(id) ;
    }
  }

  void
  qtTimeKeeper::arm(unsigned long milliseconds, bool r) {
    // std::cerr << "qtTimeKeeper::arm " << milliseconds << " " << r << std::endl ;
    if (_state!=DISARMED) killTimer(id) ;
    triggertime = TimeStamp::createAsInt() + milliseconds ;
    id = startTimer(milliseconds) ;
    repeat = r ;
    _state = ARMED ;
  }

  long
  qtTimeKeeper::getTimeLeft(void) {
    // std::cerr << "qtTimeKeeper::getTimeLeft" << std::endl ;
    if (_state==DISARMED) return -1 ;
    if (_state==TRIGGERED) return 0 ;
    TimeStamp::inttype timeleft = triggertime - TimeStamp::createAsInt() ;
    return (timeleft<0) ? 0 : timeleft ;
  }

  void
  qtTimeKeeper::disarm(void) {
    // std::cerr << "qtTimeKeeper::disarm" << std::endl ;
    killTimer(id) ;
    _state = DISARMED ;
  }

}
