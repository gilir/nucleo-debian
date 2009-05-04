#ifndef n_qtFileKeeper_H
#define n_qtFileKeeper_H

/*
 *
 * nucleo/plugins/qt/qtFileKeeper.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/FileKeeper.H>

#include <QObject>
#include <QSocketNotifier>

namespace nucleo {

  class qtFileKeeper : public QObject, public FileKeeper {

  Q_OBJECT
  
  protected:
    
    QSocketNotifier *rsn, *wsn, *esn ;
    
    void cleanup(void) ;
				   
  protected Q_SLOTS:
    
    void callback(void) ;
    
  public:
    
    qtFileKeeper(void) ;
    ~qtFileKeeper(void) ;
    
    void setup(int fd, eventMask mask) ;
  
  } ;

}

#endif
