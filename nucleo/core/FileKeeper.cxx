/*
 *
 * nucleo/core/FileKeeper.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/FileKeeper.H>

#include <sys/select.h>

namespace nucleo {
  
  FileKeeper*
  FileKeeper::create(void) {
    return ReactiveEngine::createFileKeeper() ;
  }
  
  FileKeeper*
  FileKeeper::create(int fd, eventMask mask) {
    FileKeeper *fk = ReactiveEngine::createFileKeeper() ;
    fk->setup(fd, mask) ;
    return fk ;
  }
  
  FileKeeper::eventMask
  FileKeeper::getState(void) {
    if (_fd==-1) return FileKeeper::NOTHING ;

    fd_set rfds, wfds, efds ;
    FD_ZERO(&rfds) ;
    FD_ZERO(&wfds) ;
    FD_ZERO(&efds) ;
    if (_mask&R) FD_SET(_fd, &rfds) ;
    if (_mask&W) FD_SET(_fd, &wfds) ;
    if (_mask&E) FD_SET(_fd, &efds) ;

    int state = FileKeeper::NOTHING ;

    struct timeval tv ;
    tv.tv_sec = tv.tv_usec = 0 ;
    if (select(_fd+1,&rfds,&wfds,&efds,&tv)) {
	 if (FD_ISSET(_fd, &rfds)) state = state|R ;
	 if (FD_ISSET(_fd, &wfds)) state = state|W ;
	 if (FD_ISSET(_fd, &efds)) state = state|E ;
    }

    return (eventMask)state ;
  }

}
