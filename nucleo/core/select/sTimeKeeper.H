#ifndef n_sTimeKeeper_H
#define n_sTimeKeeper_H

/*
 *
 * nucleo/core/select/sTimeKeeper.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/select/sReactiveEngine.H>

#include <nucleo/core/TimeKeeper.H>
#include <nucleo/core/TimeStamp.H>

namespace nucleo {

  // ---------------------------------------------------------------

  class sTimeKeeper : public TimeKeeper {

  protected:

    friend class sReactiveEngine ;
    
    bool _repeat ;
    TimeStamp::inttype _epoch, _delay ;

    static std::list<sTimeKeeper*> _instances ;

    void _checkTime(void) ;

  public:

    sTimeKeeper() ;
    ~sTimeKeeper() ;

    void arm(unsigned long milliseconds, bool repeat=false) ;
    void disarm(void) ;
    long getTimeLeft(void) ;

  } ;

  // ---------------------------------------------------------------

}

#endif
