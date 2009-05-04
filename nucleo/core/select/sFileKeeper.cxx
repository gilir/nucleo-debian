/*
 *
 * nucleo/core/select/sFileKeeper.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/select/sFileKeeper.H>

#include <iostream>

namespace nucleo {

  std::list<sFileKeeper*> sFileKeeper::_instances ;

  sFileKeeper::sFileKeeper(void) {
    _fd = -1 ;
    _instances.push_back(this) ;
    // std::cerr << "added FileKeeper " << this << std::endl ;
  }

  sFileKeeper::~sFileKeeper() {
    _instances.remove(this) ;
    _fd = -1 ;
    // std::cerr << "removed FileKeeper " << this << std::endl ;
  }

  void
  sFileKeeper::setup(int fd, eventMask mask) {
    _fd = fd ;
    _mask = mask ;
    // std::cerr << "setup FileKeeper " << this << " (" << _fd << ")" << std::endl ;
  }

}
