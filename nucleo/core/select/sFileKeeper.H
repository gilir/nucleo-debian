#ifndef n_sFileKeeper_H
#define n_sFileKeeper_H

/*
 *
 * nucleo/core/select/sFileKeeper.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/FileKeeper.H>

namespace nucleo {

  // ---------------------------------------------------------------

  class sFileKeeper : public FileKeeper {

  protected:

    friend class sReactiveEngine ;

    static std::list<sFileKeeper*> _instances ;

  public:

    sFileKeeper(void) ;
    ~sFileKeeper() ;

    void setup(int fd, eventMask mask) ;

  } ;

  // ---------------------------------------------------------------

}

#endif
