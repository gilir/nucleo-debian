#ifndef n_qtTimeKeeper_H
#define n_qtTimeKeeper_H

/*
 *
 * nucleo/plugins/qt/qtTimeKeeper.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/TimeKeeper.H>
#include <nucleo/core/TimeStamp.H>

#include <QObject>

namespace nucleo {

  class qtTimeKeeper : public QObject, public TimeKeeper {

  Q_OBJECT

  protected:

    TimeStamp::inttype triggertime ;
    bool repeat ;
    int id ;
    void timerEvent(QTimerEvent *event) ;

  public:

    void arm(unsigned long milliseconds, bool repeat=false) ;
    long getTimeLeft(void) ;
    void disarm(void) ;

  } ;

}

#endif
